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

#include <black/sat/cnf.hpp>
#include <black/logic/alphabet.hpp>

#include <tsl/hopscotch_set.h>

namespace black::internal 
{
  std::vector<clause> tseitin(formula f);
  void tseitin(
    formula f, std::vector<clause> &clauses, tsl::hopscotch_set<formula> &memo
  );

  // TODO: disambiguate fresh variables
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    atom a = f.alphabet()->var(f);
    return a;
  }

  cnf to_cnf(formula f) {
    return tseitin(f);
  }

  std::vector<clause> tseitin(formula f) {
    std::vector<clause> result;
    tsl::hopscotch_set<formula> memo;
    
    formula simple = simplify_deep(f);
    black_assert(simple.is<boolean>() || !has_constants(simple));

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

    return result;
  }

  void tseitin(
    formula f, std::vector<clause> &clauses, tsl::hopscotch_set<formula> &memo
  ) {
    if(memo.find(f) != memo.end())
      return;

    memo.insert(f);
    f.match(
      [](boolean) { },
      [](atom)  {  },
      [&](conjunction, formula l, formula r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for conjunctions:
        //   f <-> (l ∧ r) == (!f ∨ l) ∧ (!f ∨ r) ∧ (!l ∨ !r ∨ f)
        clauses.insert(end(clauses), {
          {{false, fresh(f)}, {true, fresh(l)}},
          {{false, fresh(f)}, {true, fresh(r)}},
          {{false, fresh(l)}, {false, fresh(r)}, {true, fresh(f)}}
        });
      },
      [&](disjunction, formula l, formula r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for disjunctions:
        //   f <-> (l ∨ r) == (f ∨ !l) ∧ (f ∨ !r) ∧ (l ∨ r ∨ !f)
        clauses.insert(end(clauses), {
          {{true, fresh(f)}, {false, fresh(l)}},
          {{true, fresh(f)}, {false, fresh(r)}},
          {{true, fresh(l)}, {true, fresh(r)}, {false, fresh(f)}}
        });
      },
      [&](implication, formula l, formula r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for double implications:
        //    f <-> (l -> r) == (!f ∨ !l ∨ r) ∧ (f ∨ l) ∧ (f ∨ !r)
        clauses.insert(end(clauses), {
          {{false, fresh(f)}, {false, fresh(l)}, {true, fresh(r)}},
          {{true,  fresh(f)}, {true,  fresh(l)}},
          {{true,  fresh(f)}, {false, fresh(r)}}
        });     
      },
      [&](iff, formula l, formula r) 
      {
        tseitin(l, clauses, memo);
        tseitin(r, clauses, memo);

        // clausal form for double implications:
        //    f <-> (l <-> r) == (!f ∨ !l ∨  r) ∧ (!f ∨ l ∨ !r) ∧
        //                       ( f ∨ !l ∨ !r) ∧ ( f ∨ l ∨  r)
        clauses.insert(end(clauses), {
          {{false, fresh(f)}, {false, fresh(l)}, {true,  fresh(r)}},
          {{false, fresh(f)}, {true,  fresh(l)}, {false, fresh(r)}},
          {{true,  fresh(f)}, {false, fresh(l)}, {false, fresh(r)}},
          {{true,  fresh(f)}, {true,  fresh(l)}, {true,  fresh(r)}}
        });
      },
      [&](negation, formula arg) {
        return arg.match(
          [&](boolean) { },
          [&](atom a) {
            // clausal form for negations:
            // f <-> !p == (!f ∨ !p) ∧ (f ∨ p)
            clauses.insert(end(clauses), {
              {{false, fresh(f)}, {false, fresh(a)}},
              {{true,  fresh(f)}, {true,  fresh(a)}}
            });
          },
          [&](negation, formula op) {
            tseitin(op, clauses, memo);
          },
          [&](conjunction, formula l, formula r) {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated conjunction:
            //   f <-> !(l ∧ r) == (!f ∨ !l ∨ !r) ∧ (f ∨ l) ∧ (f ∨ r)
            clauses.insert(end(clauses), {
              {{false, fresh(f)}, {false, fresh(l)}, {false, fresh(r)}},
              {{true,  fresh(f)}, {true, fresh(l)}},
              {{true,  fresh(f)}, {true, fresh(r)}},
            });
          },
          [&](disjunction, formula l, formula r) {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated disjunction:
            //   f <-> !(l ∨ r) == (f ∨ l ∨ r) ∧ (!f ∨ !l) ∧ (!f ∨ !r)
            clauses.insert(end(clauses), {
              {{true,  fresh(f)}, {true,  fresh(l)}, {true, fresh(r)}},
              {{false, fresh(f)}, {false, fresh(l)}},
              {{false, fresh(f)}, {false, fresh(r)}},
            });
          },
          [&](implication, formula l, formula r) 
          {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated implication:
            //   f <-> (l ∧ r) == (!f ∨ l) ∧ (!f ∨ !r) ∧ (!l ∨ r ∨ f)
            clauses.insert(end(clauses), {
              {{false, fresh(f)}, {true, fresh(l)}},
              {{false, fresh(f)}, {false, fresh(r)}},
              {{false, fresh(l)}, {true, fresh(r)}, {true, fresh(f)}}
            });
          },
          [&](iff, formula l, formula r) {
            tseitin(l, clauses, memo);
            tseitin(r, clauses, memo);

            // clausal form for negated double implication (xor):
            //    f <-> !(l <-> r) == (!f ∨ !l ∨ !r) ∧ (!f ∨  l ∨ r) ∧
            //                        (f  ∨  l ∨ !r) ∧ (f  ∨ !l ∨ r)
            clauses.insert(end(clauses), {
              {{false, fresh(f)}, {false, fresh(l)}, {false, fresh(r)}},
              {{false, fresh(f)}, {true,  fresh(l)}, {true,  fresh(r)}},
              {{true,  fresh(f)}, {true,  fresh(l)}, {false, fresh(r)}},
              {{true,  fresh(f)}, {false, fresh(l)}, {true,  fresh(r)}}
            });
          },
          [](temporal) { black_unreachable(); }
        );
      },
      [](temporal) { black_unreachable(); }
    );
  }

  formula to_formula(literal lit) {
    return lit.sign ? formula{lit.atom} : formula{!lit.atom};
  }

  formula to_formula(alphabet &sigma, clause c) {
    if(c.literals.empty())
      return sigma.bottom();
    
    formula f = to_formula(c.literals.front());
    for(size_t i = 1; i < c.literals.size(); ++i)
      f = f || to_formula(c.literals[i]);

    return f;
  }

  formula to_formula(alphabet &sigma, cnf c) {
    if(c.clauses.empty())
      return sigma.bottom();
    
    formula f = to_formula(sigma, c.clauses.front());
    for(size_t i = 1; i < c.clauses.size(); ++i)
      f = f && to_formula(sigma, c.clauses[i]);

    return f;
  }
}
