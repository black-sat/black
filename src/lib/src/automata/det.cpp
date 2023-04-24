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

#include <tsl/hopscotch_set.h>
#include <tsl/hopscotch_map.h>

namespace black_internal {

  namespace bdd = black::bdd;

  struct det_t {

    det_t(automaton _aut) 
      : aut{std::move(_aut)}, 
        mgr{aut.manager}, 
        sigma{*mgr->sigma()},
        _eps{mgr->variable(freshed(sigma.proposition("eps")))},
        T_eps{mgr->top(), mgr->top(), mgr->top()} { }

    size_t other(size_t primes);

    bdd::variable eps();
    bdd::node make_t_eps();
    std::pair<bdd::node, bdd::node> T_step(
      bdd::node last, bdd::node lastp, size_t primes
    );
    bdd::node trans(bdd::node t_kp, bdd::node t_k);
    bool is_total(bdd::node t_quot);
    std::vector<black::proposition> vars(bdd::node trans);
    bdd::node init(std::vector<black::proposition> const &vars);
    bdd::node finals(bdd::node t_k, size_t primes);
    automaton semideterminize();

    automaton aut;
    bdd::manager *mgr;
    black::alphabet &sigma;
    bdd::variable _eps;
    bdd::node T_eps[3];
  };

  size_t det_t::other(size_t p) {
    return 3 - p; // 3 - 1 = 2, 3 - 2 = 1
  }

  bdd::variable det_t::eps() {
    return _eps;
  }

  bdd::node det_t::make_t_eps() {
    bdd::node frame = big_and(mgr, aut.variables, [&](auto prop) {
      bdd::variable var = mgr->variable(prop);
      return iff(var, prime(var));
    });

    return (eps() && frame) || (!eps() && aut.trans);
  }

  std::pair<bdd::node, bdd::node> det_t::T_step(
    bdd::node lastp, bdd::node last, size_t primes
  ) {
    tsl::hopscotch_map<black::proposition, black::proposition> freshes;
    for(auto p : aut.letters)
      freshes.insert({p, freshed(p)});
    
    auto f = make_renamer([&](black::proposition p) {
      if(freshes.contains(p))
        return freshes.at(p);
      return p;
    });

    bdd::node tp = 
      exists(primed(other(primes)) * aut.variables,
        lastp[aut.letters / (f | primed())] && T_eps[primes]
      );
    bdd::node t =
      exists(primed(other(primes)) * aut.variables,
        last[aut.letters / f] && T_eps[primes]
      );

    return std::pair{tp, t};
  }

  bdd::node det_t::trans(bdd::node t_kp, bdd::node t_k) {
    return 
      forall(of_kind(aut.variables),
        iff(
          t_kp.condition(aut.letters, true),
          t_k.condition(eps(), false)
        )
      );
  }

  bool det_t::is_total(bdd::node trans) {
    return forall(of_kind(aut.variables), 
      forall(!of_kind(aut.variables),
        exists(primed(), trans) 
      )
    ).is_one();
  }

  std::vector<black::proposition> det_t::vars(bdd::node trans) {
    tsl::hopscotch_set<black::proposition> result;
    auto varf = of_kind(aut.variables);
    auto lettersf = of_kind(aut.letters);

    for(auto v : trans.variables()) {
      auto p = v.name();
      if(!varf.filter(p) && !lettersf.filter(p))
        result.insert(plain(p));
    }

    return std::vector<black::proposition>(begin(result), end(result));
  }

  bdd::node det_t::init(std::vector<black::proposition> const &vars) {
    return big_and(mgr, vars, [&](auto p) {
      return mgr->variable(p);
    });
  }
  
  bdd::node det_t::finals(bdd::node t_k, size_t primes) {
    return exists(of_kind(aut.variables),
      aut.init && 
      t_k.condition(aut.letters, true) && 
      aut.finals[aut.variables / primed(primes)]
    );
  }

  automaton det_t::semideterminize() 
  {
    aut.letters.push_back(eps().name());

    std::cerr << "semideterminizing... k = 1" << std::flush;

    T_eps[0] = make_t_eps();
    T_eps[1] = T_eps[0][aut.variables / primed(2)];
    T_eps[2] = T_eps[0][primed(1) * aut.variables / primed(2)]
                       [aut.variables / primed(1)];

    size_t k = 1;
    size_t primes = 1;
    bdd::node t_k = T_eps[0];
    bdd::node t_kp = T_eps[0];
    bdd::node trans = mgr->top();

    do {
      k++;
      primes = other(primes);

      std::cerr << ", " << k << std::flush;

      std::tie(t_kp, t_k) = T_step(t_kp, t_k, primes);

      trans = this->trans(t_kp, t_k);

    } while(!is_total(trans));

    std::cerr << ", computing init and finals... " << std::flush;

    std::vector<black::proposition> vars = this->vars(trans);
    bdd::node init = this->init(vars);
    bdd::node finals = this->finals(t_k, primes);

    std::cerr << "done!\n";

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