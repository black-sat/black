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

  struct eps_t { 
    bool operator==(eps_t const&) const = default;
  };

  inline std::string to_string(eps_t) {
    return "|eps|";
  }

}

template<>
struct std::hash<black_internal::eps::eps_t> {
  size_t operator()(black_internal::eps::eps_t) const {
    return 0;
  }
};

namespace black_internal {

  namespace sdd = black::sdd;

  struct det_t {

    det_t(automaton _aut) 
      : aut{std::move(_aut)}, mgr{_aut.manager}, sigma{*mgr->sigma()} { 
      aut.letters.push_back(eps().name());
    }

    sdd::variable eps();
    sdd::node T_eps();
    sdd::node T_step(sdd::node last, size_t k);
    sdd::node T_quot(sdd::node t_k1, sdd::node t_k, size_t k);
    bool is_total(sdd::node t_quot);
    std::vector<black::proposition> variables(sdd::node trans);
    sdd::node init(size_t k);
    sdd::node finals(sdd::node t_k);
    automaton semideterminize();

    automaton aut;
    sdd::manager *mgr;
    black::alphabet &sigma;
  };

  sdd::variable det_t::eps() {
    return mgr->variable(mgr->sigma()->proposition("|eps|"));
  }

  sdd::node det_t::T_eps() {
    sdd::node equals = mgr->top();
    for(auto prop : aut.variables) {
      sdd::variable var = mgr->variable(prop);
      equals = equals && iff(var, prime(var, 1));
    }
    return (implies(eps(), equals) && implies(!eps(), aut.trans));
  }

  sdd::node det_t::T_step(sdd::node last, size_t k) {
    if(k == 0)
      return T_eps()[any_of(aut.letters) / stepped(0)];

    return exists(primed(2),
      last[primed(1) * any_of(aut.variables) / primed(2)] && 
      aut.trans[any_of(aut.variables) / primed(2)]
               [any_of(aut.letters) / stepped(k)]
    );
  }

  sdd::node det_t::T_quot(sdd::node t_k1, sdd::node t_k, size_t k) {
    return 
      forall(any_of(aut.variables),
        forall(primed(1) * any_of(aut.variables),
          iff(
            t_k,
            t_k1[stepped(k + 1) / plain()].condition(!step(eps(), k + 1))
                [stepped() / primed()]
          )
        )
      );
  }

  bool det_t::is_total(sdd::node t_quot) {
    return exists(primed() * stepped(), t_quot).is_valid();
  }

  std::vector<black::proposition> det_t::variables(sdd::node trans) {
    std::vector<black::proposition> result;
    std::vector<sdd::variable> vars;

    black_assert(std::find(begin(vars), end(vars), eps()) == end(vars));

    for(auto var : trans.variables())
      result.push_back(var.name());

    return result;
  }

  sdd::node det_t::init(size_t k) {
    return big_and(mgr, range(0, k + 1), [&](size_t i) {
      return step(eps(), i);
    });
  }
  
  sdd::node det_t::finals(sdd::node t_k) {
    return exists(any_of(aut.variables),
      exists(primed(1) * any_of(aut.variables),
        aut.init && t_k && aut.finals[any_of(aut.variables) / primed(1)]
      )
    );
  }

  automaton det_t::semideterminize() {
    sdd::node trans = mgr->top();
    sdd::node t_k = mgr->top();
    sdd::node t_k1 = T_step(mgr->top(), 0);
    
    std::cerr << "Start semi-determinization... " << std::flush;
    size_t k = 0;
    do {
      std::cerr << "k = " << k << "\n";
      t_k = t_k1;
      t_k1 = T_step(t_k, k + 1);
      
      trans = T_quot(t_k1, t_k, k);

      k++;
    } while(!is_total(trans));

    std::cerr << "done!\n";

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