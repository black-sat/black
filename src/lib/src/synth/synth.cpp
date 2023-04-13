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

#include <black/synth/synth.hpp>
#include <black/sdd/sdd.hpp>

#include <iostream>
#include <algorithm>

namespace black_internal::synth {

  namespace sdd = black::sdd;
  using formula = black::logic::fragments::propositional::formula;

  struct synth_t 
  {
    synth_t(automata_spec _spec) 
      : spec{_spec}, aut{cover(spec.spec)}, mgr{aut.manager} { }

    black::proposition cover(black::proposition p);
    formula cover(formula f);
    sdd::node cover(sdd::node n);
    automaton cover(automaton a);

    sdd::node unravel(sdd::node last, size_t k);
    sdd::node winC(sdd::node last, size_t k);
    sdd::node winE(sdd::node last, size_t k);
    sdd::node encodeC(sdd::node n_unravel, sdd::node n_win, size_t n);
    sdd::node encodeE(sdd::node n_unravel, sdd::node n_win, size_t n);

    black::tribool is_realizable();  

    automata_spec spec;
    automaton aut;
    sdd::manager *mgr;
    std::unordered_map<formula, formula> cover_cache;
  };

  black::proposition synth_t::cover(black::proposition p) {
    return p.sigma()->proposition(p);
  }

  formula synth_t::cover(formula f) {
    using namespace black::logic::fragments::propositional;
    if(cover_cache.contains(f))
      return cover_cache.at(f);

    formula result = f.match(
      [](boolean b) { return b; },
      [&](proposition p) { return cover(p); },
      [&](unary u, auto arg) {
        return unary(u.node_type(), cover(arg));
      },
      [&](conjunction c) {
        return big_and(*c.sigma(), c.operands(), [&](auto op) {
          return cover(op);
        });
      },
      [&](disjunction c) {
        return big_or(*c.sigma(), c.operands(), [&](auto op) {
          return cover(op);
        });
      },
      [&](binary b, auto left, auto right) {
        return binary(b.node_type(), cover(left), cover(right));
      }
    );

    cover_cache.insert({f, result});
    return result;
  }

  sdd::node synth_t::cover(sdd::node n) {
    return n.manager()->to_node(cover(n.manager()->to_formula(n)));
  }

  automaton synth_t::cover(automaton a) {
    std::transform(begin(a.letters), end(a.letters), begin(a.letters), 
      [&](auto p) {
        return cover(p);
      }
    );
    std::transform(begin(a.variables), end(a.variables), begin(a.variables), 
      [&](auto p) {
        return cover(p);
      }
    );

    a.init = cover(a.init);
    a.trans = cover(a.trans);
    a.finals = cover(a.finals);

    return a;
  }

  sdd::node synth_t::unravel(sdd::node last, size_t k) {
    if(k == 0)
      return aut.init[any_of(aut.variables) / stepped(0)];
    
    return 
      last && 
      aut.trans[any_of(aut.variables) / stepped(k)]
               [primed() * any_of(aut.variables) / stepped(k+1)];
  }

  sdd::node synth_t::winC(sdd::node last, size_t k) {
    return last || aut.finals[any_of(aut.variables) / stepped(k)];
  }

  sdd::node synth_t::winE(sdd::node last, size_t k) {
    using namespace black::logic::fragments::propositional;
    return last || (
      big_or(mgr, range(0, k), [&](size_t j) {
        return big_and(mgr, aut.letters, [&](auto p) {
          return mgr->to_node(iff(step(p, j), step(p, k)));
        });
      }) &&
      big_and(mgr, range(0, k+1), [&](size_t w) {
        return !aut.finals[any_of(aut.variables) / stepped(w)];
      })
    );
  }

  sdd::node synth_t::encodeC(sdd::node n_unravel, sdd::node n_win, size_t n) {
    sdd::node result = 
      exists(stepped(n) * any_of(aut.variables), n_unravel && n_win);

    for(size_t i = 0; i < n; i++) {
      size_t k = n - i - 1;
      
      result = exists(stepped(k) * any_of(aut.variables),
        exists(stepped(k) * any_of(spec.outputs), 
          forall(stepped(k) * any_of(spec.inputs),
            result
          )
        )
      );
    }

    return result;
  }

  sdd::node synth_t::encodeE(sdd::node n_unravel, sdd::node n_win, size_t n) {
    sdd::node result = 
      forall(stepped(n) * any_of(aut.variables), n_unravel && n_win);

    for(size_t i = 0; i < n; i++) {
      size_t k = n - i - 1;
      
      result = forall(stepped(k) * any_of(aut.variables),
        forall(stepped(k) * any_of(spec.outputs), 
          exists(stepped(k) * any_of(spec.inputs),
            result
          )
        )
      );
    }

    return result;
  }

  black::tribool synth_t::is_realizable() {
    black_assert(mgr);
    sdd::node n_unravel = mgr->top();
    sdd::node n_winC = mgr->bottom();
    sdd::node n_winE = mgr->bottom();

    size_t n = 0;
    while(true) {
      std::cerr << "n = " << n << "\n";
      
      n_unravel = unravel(n_unravel, n);
      n_winC = winC(n_winC, n);
      n_winE = winE(n_winE, n);
      std::cerr << " - unravel: " << n_unravel.count() << "\n";
      std::cerr << " - winC: " << n_winC.count() << "\n";
      std::cerr << " - winE: " << n_winE.count() << "\n";
      
      sdd::node testC = encodeC(n_unravel, n_winC, n);
      if(testC.is_valid())
        return true;
      
      sdd::node testE = encodeE(n_unravel, n_winE, n);
      if(testE.is_valid())
        return true;

      n++; 
    }
  }


  black::tribool is_realizable(automata_spec const& spec) {
    return synth_t{spec}.is_realizable();
  }


}