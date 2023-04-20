//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <black/automata/automaton.hpp>
#include <black/support/range.hpp>

#include <iostream>
#include <cmath>

namespace black_internal::eps {

  struct fresh_t { 
    std::string name;
    bool operator==(fresh_t const&) const = default;
  };

  inline std::string to_string(fresh_t f) {
    return "|" + f.name + "|";
  }

}

template<>
struct std::hash<black_internal::eps::fresh_t> {
  size_t operator()(black_internal::eps::fresh_t f) const {
    return std::hash<std::string>{}(f.name);
  }
};

namespace black_internal {

  namespace sdd = black::sdd;

  struct det_t {

    det_t(automaton _aut) 
      : aut{std::move(_aut)}, 
        mgr{aut.manager}, 
        sigma{*mgr->sigma()},
        t_eps{T_eps()} { }

    sdd::variable eps();
    sdd::node T_eps();
    sdd::node T_step(sdd::node last, size_t k);
    sdd::node trans(sdd::node t_k);
    bool is_total(sdd::node t_quot);
    sdd::node init(size_t k);
    sdd::node finals(sdd::node t_k);
    automaton semideterminize();

    automaton aut;
    sdd::manager *mgr;
    black::alphabet &sigma;
    sdd::node t_eps;
    std::optional<sdd::node> t_eps2;
  };

  sdd::variable det_t::eps() {
    return mgr->variable(mgr->sigma()->proposition(eps::fresh_t{"eps"}));
  }

  sdd::node det_t::T_eps() {
    sdd::node frame = big_and(mgr, aut.variables, [&](auto prop) {
      sdd::variable var = mgr->variable(prop);
      return iff(var, prime(var));
    });

    return (eps() && frame) || (!eps() && aut.trans);
  }

  sdd::node det_t::T_step(sdd::node last, size_t k) {
    black_assert(k >= 1);

    if(k == 1)
      return t_eps;

    if(!t_eps2)
      t_eps2 = t_eps[aut.variables / primed(2)];

    return exists(primed(2),
      last[aut.letters / stepped(k - 2)]
          [primed(1) * aut.variables / primed(2)] && *t_eps2
    );
  }

  sdd::node det_t::trans(sdd::node t_k) {
    return forall(of_kind(aut.variables),
      iff(
        t_k.condition(!eps()),
        t_k.condition(aut.letters, true)[stepped() / primed()]
      )
    );
  }

  [[maybe_unused]]
  static void enumerate(sdd::node n) {
    while(!n.is_unsat()) {
      auto model = n.model();
      black_assert(model.has_value());
      std::cerr << " - " << *model << "\n";
      n = n && !big_and(n.manager(), *model, [](auto lit) { return lit; });
    }
  }

  bool det_t::is_total(sdd::node trans) {
    return exists(primed() * stepped(), trans).is_valid();
  }

  sdd::node det_t::init(size_t k) {
    return big_and(mgr, range(0, k - 1), [&](size_t i) {
      return step(eps(), i);
    });
  }
  
  sdd::node det_t::finals(sdd::node t_k) {
    return exists(of_kind(aut.variables),
      aut.init && 
      t_k.condition(aut.letters, true) && 
      aut.finals[aut.variables / primed(1)]
    );
  }

  automaton det_t::semideterminize() 
  {
    aut.letters.push_back(eps().name());

    //std::cerr << "Starting semi-determinization... " << std::flush;
    
    // std::cerr << "automaton:\n";
    // std::cerr << " - init: " 
    //           << black::to_string(mgr->to_formula(aut.init)) << "\n";
    // std::cerr << " - trans: " 
    //           << black::to_string(mgr->to_formula(aut.trans)) << "\n";
    // std::cerr << " - finals: " 
    //           << black::to_string(mgr->to_formula(aut.finals)) << "\n";


    size_t k = 1;
    sdd::node t_k = T_step(mgr->top(), k);
    sdd::node trans = mgr->top();
    
    do {
      k++;
      if(k == 2)
        std::cerr << "k = " << k << std::flush;
      else 
        std::cerr << ", " << k << std::flush;

      sdd::node prev = t_k;
      t_k = T_step(t_k, k);

      if(iff(prev, t_k).is_valid()) {
        std::cerr << "T_step did not evolve!\n";
        std::cerr << "- variables:\n";
        for(auto v : t_k.variables())
          std::cerr << "   - " << black::to_string(v.name()) << "\n";
        throw std::runtime_error("t_k did not evolve");
      }

      trans = this->trans(t_k);

    } while(!is_total(trans));

    std::cerr << ", computing init and finals... " << std::flush;

    sdd::node init = this->init(k);
    sdd::node finals = this->finals(t_k);

    std::cerr << "done!\n";

    std::vector<black::proposition> vars;
    for(size_t i = 0; i < k - 1; i++) {
      for(auto p : aut.letters) {
        vars.push_back(step(p, i));
        vars.push_back(prime(step(p, i)));
      }
    }

    aut.letters.pop_back(); // remove eps()
    return automaton {
      .manager = aut.manager,
      .letters = aut.letters,
      .variables = vars,
      .init = init,
      .trans = trans,
      .finals = finals
    };
  }

  automaton semideterminize(automaton aut) {
    return det_t{aut}.semideterminize();
  }

}