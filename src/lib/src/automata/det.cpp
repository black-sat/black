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

    det_t(automaton _aut) : aut{std::move(_aut)}, mgr{_aut.manager} { 
      aut.letters.push_back(eps().name());
    }

    sdd::variable eps();
    sdd::node T_eps();
    sdd::node T_step(sdd::node last, size_t n);
    automaton determinize();

    automaton aut;
    sdd::manager *mgr;
  };

  sdd::variable det_t::eps() {
    return mgr->variable(mgr->sigma()->proposition("|eps|"));
  }

  sdd::node det_t::T_eps() {
    sdd::node equals = mgr->top();
    for(auto prop : aut.variables) {
      sdd::variable var = mgr->variable(prop);
      equals = equals && iff(var, make_primed(var, 1));
    }

    return (implies(eps(), equals) && implies(!eps(), aut.trans));
  }

  sdd::node det_t::T_step(sdd::node last, size_t n) {
    if(n == 0)
      return T_eps()[any_of(aut.letters) / stepped(0)];

    std::vector<black::proposition> temps = aut.variables; // TODO
    
    return exists(primed(2),
      last[primed(1) / primed(2)] && 
      aut.trans[any_of(aut.variables) / primed(2)]
               [any_of(aut.letters) / stepped(n)]
    );
  }

  automaton det_t::determinize() {
    sdd::node last = mgr->top();
    for(size_t i = 0; i < 200; i++) {
      std::cerr << "- k = " << i << "\n";
      last = T_step(last, i);
    }

    aut.trans = last;
    return aut;
  }

  automaton determinize(automaton aut) {
    return det_t{aut}.determinize();
  }

}