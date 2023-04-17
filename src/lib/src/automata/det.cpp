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

    automaton totalize(automaton t);
    sdd::variable eps();
    sdd::variable x_sink();
    sdd::node T_eps();
    sdd::node T_step(sdd::node last, size_t k);
    sdd::node phi_bullet(size_t k);
    sdd::node phi_tilde(sdd::node t_k1, sdd::node t_k);
    sdd::node trans(sdd::node t_k1, sdd::node t_k, size_t k);
    bool is_total(sdd::node t_quot);
    sdd::node init(size_t k);
    sdd::node finals(sdd::node t_k);
    automaton semideterminize();

    automaton aut;
    sdd::manager *mgr;
    black::alphabet &sigma;
    sdd::node t_eps;
  };

  automaton det_t::totalize(automaton a) {
    sdd::node part = exists(primed(), a.trans);

    if(part.is_valid())
      return a;

    // a.variables.push_back(x_sink().name());
    // a.trans = 
    //   (!x_sink() && a.trans && !prime(x_sink())) || 
    //   ((x_sink() || !part) && prime(x_sink()));
    // a.init = a.init && !x_sink();
    // a.finals = a.finals && !x_sink();

    a.variables.push_back(x_sink().name());
    a.trans = 
      (a.trans || prime(x_sink())) && (implies(x_sink(), prime(x_sink())));
    a.init = a.init && !x_sink();
    a.finals = a.finals && !x_sink();

    black_assert(exists(primed(), a.trans).is_valid());

    return a;
  }

  sdd::variable det_t::eps() {
    return mgr->variable(mgr->sigma()->proposition(eps::fresh_t{"eps"}));
  }
  
  sdd::variable det_t::x_sink() {
    return mgr->variable(mgr->sigma()->proposition(eps::fresh_t{"sink"}));
  }

  sdd::node det_t::T_eps() {
    sdd::node equals = mgr->top();
    for(auto prop : aut.variables) {
      sdd::variable var = mgr->variable(prop);
      equals = equals && iff(var, prime(var));
    }
    return (eps() && equals) || (!eps() && aut.trans);
  }

  sdd::node det_t::T_step(sdd::node last, size_t k) {
    if(k == 0)
      return t_eps[aut.letters / stepped(0)];

    return exists(primed(2),
      last[primed(1) * aut.variables / primed(2)] && 
      t_eps[aut.variables / primed(2)]
           [aut.letters / stepped(k)]
    );
  }

  sdd::node det_t::phi_tilde(sdd::node t_k1, sdd::node t_k) {
    return 
      forall(aut.variables,
        forall(primed(1) * aut.variables, 
          iff(
            t_k[stepped() / primed(1)],
            t_k1[stepped() / primed(2)]
          )
        )
      );
  }

  sdd::node det_t::phi_bullet(size_t k) {
    using namespace black::logic::fragments::propositional;

    return big_and(mgr, range(0, k+1), [&](size_t i) {
      return big_and(mgr, aut.letters, [&](auto p) {
        return mgr->to_node(iff(step(p, i), prime(step(p, i), 2)));
      });
    }) &&
    !prime(step(eps(), k + 1), 2)
    &&
    big_and(mgr, aut.letters, [&](auto p){
      if(p == eps().name())
        return mgr->top();
      return mgr->to_node(iff(p, prime(step(p, k + 1), 2)));
    });
  }

  sdd::node det_t::trans(sdd::node t_k1, sdd::node t_k, size_t k) {
    return 
      exists(primed(2),
        phi_bullet(k) &&
        phi_tilde(t_k1, t_k)
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
    sdd::node result = exists(primed() * stepped(), trans);
    if(result.is_valid())
      return true;
    return false;
  }

  sdd::node det_t::init(size_t k) {
    return big_and(mgr, range(0, k + 1), [&](size_t i) {
      return step(eps(), i);
    });
  }
  
  sdd::node det_t::finals(sdd::node t_k) {
    return exists(aut.variables,
      exists(primed(1) * aut.variables,
        aut.init && t_k && aut.finals[aut.variables / primed(1)]
      )
    );
  }

  automaton det_t::semideterminize() 
  {
    aut.letters.push_back(eps().name());

    sdd::node trans = mgr->top();
    sdd::node t_k = mgr->top();
    sdd::node t_k1 = T_step(mgr->top(), 0);
    
    std::cerr << "Start semi-determinization... " << std::flush;
    size_t k = 0;
    do {
      if(k == 0)
        std::cerr << "k = " << std::flush;
      else 
        std::cerr << ", " << std::flush;
      std::cerr << k << std::flush;

      t_k = t_k1;
      t_k1 = T_step(t_k, k + 1);

      trans = this->trans(t_k1, t_k, k);
      
      k++;
    } while(!is_total(trans));

    std::cerr << ", done!\n";

    sdd::node init = this->init(k - 1);
    sdd::node finals = this->finals(t_k);

    std::vector<black::proposition> vars;
    for(size_t i = 0; i < k; i++) {
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