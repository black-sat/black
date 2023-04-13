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

#include <black/solver/encoding.hpp>

#include <black/logic/renamings.hpp>

#include <tsl/hopscotch_set.h>

#include <iostream>

namespace black_internal {

  using namespace black::logic::fragments::LTLP;
  namespace sdd = black::sdd;

  namespace {
    struct to_automaton_t 
    {
      void collect();

      static proposition ground(formula y);

      static formula lift(proposition p);

      static logic::formula<logic::propositional> snf(formula f, bool prime);

      automaton encode();

      to_automaton_t(sdd::manager *mgr, formula f) 
        : manager{mgr}, frm{f} { 
          black_assert(mgr);
        }

      sdd::manager *manager;
      formula frm;
      std::vector<proposition> variables;
      std::vector<proposition> letters;
      std::vector<tomorrow> xreqs;
      std::vector<yesterday> yreqs;
      std::vector<w_yesterday> zreqs;
    };
  } 

  void to_automaton_t::collect() 
  {
    tsl::hopscotch_set<proposition> _variables;
    tsl::hopscotch_set<proposition> _letters;
    tsl::hopscotch_set<tomorrow> _xreqs;
    tsl::hopscotch_set<yesterday> _yreqs;
    tsl::hopscotch_set<w_yesterday> _zreqs;

    _variables.insert(ground(X(frm)));
    _xreqs.insert(X(frm));
    
    transform(frm, [&](auto child) {
      child.match(
        [&](proposition p) {
          _letters.insert(p);
        },
        [&](tomorrow x) {
          _variables.insert(ground(x));
          _xreqs.insert(x);
        },
        [&](w_tomorrow wx) {
          _variables.insert(ground(wx));
        },
        [&](yesterday y) {
          _variables.insert(ground(y));
          _yreqs.insert(y);
          _xreqs.insert(X(!y));
        },
        [&](w_yesterday z) {
          _variables.insert(ground(z));
          _zreqs.insert(z);
          _xreqs.insert(X(z));
        },
        [&](until u) {
          _variables.insert(ground(X(u)));
          _xreqs.insert(X(u));
        },
        [&](release r) {
          _variables.insert(ground(wX(r)));
        },
        [&](w_until u) {
          _variables.insert(ground(wX(u)));
        },
        [&](s_release r) {
          _variables.insert(ground(X(r)));
          _xreqs.insert(X(r));
        },
        [&](eventually f) {
          _variables.insert(ground(X(f)));
          _xreqs.insert(X(f));
        },
        [&](always g) {
          _variables.insert(ground(wX(g)));
        },
        [&](since s) {
          _variables.insert(ground(Y(s)));
          _yreqs.insert(Y(s));
          _xreqs.insert(X(!Y(s)));
        },
        [&](triggered s) {
          _variables.insert(ground(Z(s)));
          _zreqs.insert(Z(s));
          _xreqs.insert(X(Z(s)));
        },
        [&](once o) {
          _variables.insert(ground(Y(o)));
          _yreqs.insert(Y(o));
          _xreqs.insert(X(!Y(o)));
        },
        [&](historically h) {
          _variables.insert(ground(Z(h)));
          _zreqs.insert(Z(h));
          _xreqs.insert(X(Z(h)));
        },
        [](otherwise) { }
      );
    });

    variables.insert(variables.begin(), _variables.begin(), _variables.end());
    letters.insert(letters.begin(), _letters.begin(), _letters.end());
    xreqs.insert(xreqs.begin(), _xreqs.begin(), _xreqs.end());
    yreqs.insert(yreqs.begin(), _yreqs.begin(), _yreqs.end());
    zreqs.insert(zreqs.begin(), _zreqs.begin(), _zreqs.end());
  }

  proposition to_automaton_t::ground(formula f) {
    return f.sigma()->proposition(f);
  }

  formula to_automaton_t::lift(proposition p) {
    auto name = p.name().to<formula>();
    black_assert(name);
    
    return *name;
  }

  logic::formula<logic::propositional>
  to_automaton_t::snf(formula f, bool p) {
    auto _primed = [&](proposition prop){ 
      return p ? prime(prop, 1) : prop;
    };
    return f.match(
      [](boolean b) { return b; },
      [](proposition prop) { return prop; },
      [&](negation, auto arg) { 
        return !snf(arg, p);
      },
      [&](disjunction d) {
        return big_or(*f.sigma(), d.operands(), [&](auto op) {
          return snf(op, p);
        });
      },
      [&](conjunction c) {
        return big_and(*f.sigma(), c.operands(), [&](auto op) {
          return snf(op, p);
        });
      },
      [&](tomorrow y) {
        return _primed(ground(y));
      },
      [&](w_tomorrow z) {
        return _primed(ground(z));
      },
      [&](yesterday y) {
        return _primed(ground(y));
      },
      [&](w_yesterday z) {
        return _primed(ground(z));
      },
      [&](eventually, auto arg) {
        return snf(arg, p) || snf(X(F(arg)), p);
      },
      [&](always, auto arg) {
        return snf(arg, p) && snf(wX(G(arg)), p);
      },
      [&](once, auto arg) {
        return snf(arg, p) || snf(Y(O(arg)), p);
      },
      [&](historically, auto arg) {
        return snf(arg, p) && snf(Z(H(arg)), p);
      },
      [&](until, auto left, auto right) {
        return snf(right, p) || (snf(left, p) && snf(X(U(left, right)), p));
      },
      [&](release, auto left, auto right) {
        return snf(right, p) && (snf(left, p) || snf(wX(R(left, right)), p));
      },
      [&](w_until, auto left, auto right) {
        return snf(right, p) || (snf(left, p) && snf(wX(W(left, right)), p));
      },
      [&](s_release, auto left, auto right) {
        return snf(right, p) && (snf(left, p) || snf(X(M(left, right)), p));
      },
      [&](since, auto left, auto right) {
        return snf(right, p) || (snf(left, p) && snf(Y(S(left, right)), p));
      },
      [&](triggered, auto left, auto right) {
        return snf(right, p) && (snf(left, p) || snf(Z(T(left, right)), p));
      },
      [&](implication) -> logic::formula<logic::propositional> { 
        black_unreachable(); 
      },
      [&](iff) -> logic::formula<logic::propositional> { 
        black_unreachable(); 
      }
    );
  }
  
  automaton to_automaton_t::encode() 
  {
    alphabet &sigma = *frm.sigma();
    
    collect();

    auto init = 
      ground(X(frm)) && 
      big_and(sigma, yreqs, [](auto req) { return ground(X(!req)); }) &&
      big_and(sigma, zreqs, [](auto req) { return ground(X(req)); });

    auto finals = big_and(sigma, xreqs, [](auto req) { 
      return !ground(req); 
    });

    auto trans = big_and(sigma, variables, [](proposition x) {
      auto req = lift(x).to<unary>();
      bool primed = true;
      auto psi = req->argument();

      req->match(
        [&](yesterday) {
          primed = false;
          x = prime(x, 1);
        },
        [&](w_yesterday) {
          primed = false;
          x = prime(x, 1);
        },
        [](otherwise) { }
      );

      return logic::iff<logic::propositional>(x, snf(psi, primed));
    });

    return automaton {
      .manager = manager,
      .letters = letters,
      .variables = variables,
      .init = manager->to_node(init),
      .trans = manager->to_node(trans),
      .finals = manager->to_node(finals)
    };
  } 

  automaton to_automaton(sdd::manager *mgr, formula f) {
    logic::scope xi{*f.sigma()};
    auto nnf = encoder::encoder{f, xi, true}.to_nnf(f).to<formula>().value();

    return to_automaton_t{mgr, nnf}.encode();
  }
}