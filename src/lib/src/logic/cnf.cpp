//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#include <black/logic/cnf.hpp>
#include <black/logic/prettyprint.hpp>

#include <tsl/hopscotch_set.h>

#include <iostream>

namespace black_internal::cnf
{ 
  using namespace black::logic::fragments::propositional;

  formula remove_booleans(formula f);

  static
  formula remove_booleans(negation, formula op) 
  {
    // !true -> false, !false -> true
    if(auto b = op.to<boolean>(); b)
      return op.sigma()->boolean(!b->value()); 
    
    // !!p -> p
    if(auto nop = op.to<negation>(); nop) 
      return nop->argument();

    return !op;
  }

  static
  formula remove_booleans(conjunction, formula l, formula r) {
    alphabet &sigma = *l.sigma();
    std::optional<boolean> bl = l.to<boolean>(); 
    std::optional<boolean> br = r.to<boolean>();

    if(!bl && !br)
      return l && r;

    if(bl && !br) {
      return bl->value() ? r : sigma.bottom();
    }
    
    if(!bl && br)
      return br->value() ? l : sigma.bottom();

    return sigma.boolean(bl->value() && br->value());
  }

  static
  formula remove_booleans(disjunction, formula l, formula r) {
    alphabet &sigma = *l.sigma();
    std::optional<boolean> bl = l.to<boolean>();
    std::optional<boolean> br = r.to<boolean>();

    if(!bl && !br)
      return l || r;

    if(bl && !br)
      return bl->value() ? sigma.top() : r;
    
    if(!bl && br)
      return br->value() ? sigma.top() : l;
      
    return sigma.boolean(bl->value() || br->value());
  }

  static
  formula remove_booleans(implication, formula l, formula r) {
    alphabet &sigma = *l.sigma();
    std::optional<boolean> bl = l.to<boolean>();
    std::optional<boolean> br = r.to<boolean>();

    if(!bl && !br)
      return implies(l, r);

    if(bl && !br)
      return bl->value() ? r : sigma.top();
    
    if(!bl && br)
      return br->value() ? formula{sigma.top()} : !l;

    return sigma.boolean(!bl->value() || br->value());
  }

  static
  formula remove_booleans(iff, formula l, formula r) {
    alphabet &sigma = *l.sigma();
    std::optional<boolean> bl = l.to<boolean>();
    std::optional<boolean> br = r.to<boolean>();

    if(!bl && !br)
      return iff(l, r);

    if(bl && !br)
      return bl->value() ? r : !r;
    
    if(!bl && br)
      return br->value() ? l : !l;

    return sigma.boolean(bl->value() == br->value());
  }

  formula remove_booleans(formula f) {
    return f.match( // LCOV_EXCL_LINE
      [](boolean b)     -> formula { return b; },
      [](proposition p) -> formula { return p; },
      [](auto op, auto ...args) -> formula {
        return remove_booleans(op, remove_booleans(args)...);
      }
    );
  }

  static void tseitin(
    formula f, 
    std::vector<clause> &clauses, 
    tsl::hopscotch_set<formula> &memo
  );

  // TODO: disambiguate fresh propositions
  inline proposition fresh(formula f) {
    if(f.is<proposition>())
      return *f.to<proposition>();
    return f.sigma()->proposition(f);
  }

  cnf to_cnf(formula f) {
    std::vector<clause> result;
    tsl::hopscotch_set<formula> memo;
    
    formula simple = remove_booleans(f);
    std::cerr << "f: " << to_string(f) << "\n"
              << "simple: " << to_string(simple) << "\n";
    black_assert(
      simple.is<boolean>() || 
      !has_any_element_of(simple, syntax_element::boolean)
    ); // LCOV_EXCL_LINE

    tseitin(simple, result, memo);
    if(auto b = simple.to<boolean>(); b) {
      if(b->value())
        return result;
      else {
        result.push_back({});
        return result;
      }
    }

    result.push_back({{true, fresh(simple)}});

    return {result};
  }

  static void tseitin(
    formula f, 
    std::vector<clause> &clauses, 
    tsl::hopscotch_set<formula> &memo
  ) {
    if(memo.find(f) != memo.end())
      return;

    memo.insert(f);
    f.match(
      [](boolean)     { }, // LCOV_EXCL_LINE
      [](proposition) { },
      [&](conjunction, auto l, auto r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for conjunctions:
        //   f <-> (l ∧ r) == (!f ∨ l) ∧ (!f ∨ r) ∧ (!l ∨ !r ∨ f)
        clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
          {{false, fresh(f)}, {true, fresh(l)}},
          {{false, fresh(f)}, {true, fresh(r)}},
          {{false, fresh(l)}, {false, fresh(r)}, {true, fresh(f)}}
        });
      },
      [&](disjunction, auto l, auto r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for disjunctions:
        //   f <-> (l ∨ r) == (f ∨ !l) ∧ (f ∨ !r) ∧ (l ∨ r ∨ !f)
        clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
          {{true, fresh(f)}, {false, fresh(l)}},
          {{true, fresh(f)}, {false, fresh(r)}},
          {{true, fresh(l)}, {true, fresh(r)}, {false, fresh(f)}}
        });
      },
      [&](implication, auto l, auto r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for double implications:
        //    f <-> (l -> r) == (!f ∨ !l ∨ r) ∧ (f ∨ l) ∧ (f ∨ !r)
        clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
          {{false, fresh(f)}, {false, fresh(l)}, {true, fresh(r)}},
          {{true,  fresh(f)}, {true,  fresh(l)}},
          {{true,  fresh(f)}, {false, fresh(r)}}
        });     
      },
      [&](iff, auto l, auto r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for double implications:
        //    f <-> (l <-> r) == (!f ∨ !l ∨  r) ∧ (!f ∨ l ∨ !r) ∧
        //                       ( f ∨ !l ∨ !r) ∧ ( f ∨ l ∨  r)
        clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
          {{false, fresh(f)}, {false, fresh(l)}, {true,  fresh(r)}},
          {{false, fresh(f)}, {true,  fresh(l)}, {false, fresh(r)}},
          {{true,  fresh(f)}, {false, fresh(l)}, {false, fresh(r)}},
          {{true,  fresh(f)}, {true,  fresh(l)}, {true,  fresh(r)}}
        });
      },
      [&](negation, auto arg) {
        return arg.match(  // LCOV_EXCL_LINE
          [](boolean)    { black_unreachable(); }, // LCOV_EXCL_LINE
          [&](proposition a) {
            // clausal form for negations:
            // f <-> !p == (!f ∨ !p) ∧ (f ∨ p)
            clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
              {{false, fresh(f)}, {false, fresh(a)}},
              {{true,  fresh(f)}, {true,  fresh(a)}}
            });
          },
          [&](negation) { // LCOV_EXCL_LINE
            // NOTE: this case should never be invoked because 
            //       remove_booleans() removes double negations
            black_unreachable(); // LCOV_EXCL_LINE
          },
          [&](conjunction, auto l, auto r) {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated conjunction:
            //   f <-> !(l ∧ r) == (!f ∨ !l ∨ !r) ∧ (f ∨ l) ∧ (f ∨ r)
            clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
              {{false, fresh(f)}, {false, fresh(l)}, {false, fresh(r)}},
              {{true,  fresh(f)}, {true, fresh(l)}},
              {{true,  fresh(f)}, {true, fresh(r)}},
            });
          },
          [&](disjunction, auto l, auto r) {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated disjunction:
            //   f <-> !(l ∨ r) == (f ∨ l ∨ r) ∧ (!f ∨ !l) ∧ (!f ∨ !r)
            clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
              {{true,  fresh(f)}, {true,  fresh(l)}, {true, fresh(r)}},
              {{false, fresh(f)}, {false, fresh(l)}},
              {{false, fresh(f)}, {false, fresh(r)}},
            });
          },
          [&](implication, auto l, auto r) 
          {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated implication:
            //   f <-> (l ∧ r) == (!f ∨ l) ∧ (!f ∨ !r) ∧ (!l ∨ r ∨ f)
            clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
              {{false, fresh(f)}, {true, fresh(l)}},
              {{false, fresh(f)}, {false, fresh(r)}},
              {{false, fresh(l)}, {true, fresh(r)}, {true, fresh(f)}}
            });
          },
          [&](iff, auto l, auto r) {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated double implication (xor):
            //    f <-> !(l <-> r) == (!f ∨ !l ∨ !r) ∧ (!f ∨  l ∨ r) ∧
            //                        (f  ∨  l ∨ !r) ∧ (f  ∨ !l ∨ r)
            clauses.insert(clauses.end(), { // LCOV_EXCL_LINE
              {{false, fresh(f)}, {false, fresh(l)}, {false, fresh(r)}},
              {{false, fresh(f)}, {true,  fresh(l)}, {true,  fresh(r)}},
              {{true,  fresh(f)}, {true,  fresh(l)}, {false, fresh(r)}},
              {{true,  fresh(f)}, {false, fresh(l)}, {true,  fresh(r)}}
            });
          }
        );
      }
    );
  }

  formula to_formula(literal lit) {
    return lit.sign ? formula{lit.prop} : formula{!lit.prop};
  }

  formula to_formula(alphabet &sigma, clause c) {
    return big_or(sigma, c.literals, [](literal lit){
      return to_formula(lit);
    });
  }

  formula to_formula(alphabet &sigma, cnf c) {
    return big_and(sigma, c.clauses, [&](clause cl) {
      return to_formula(sigma, cl);
    });
  }
}
