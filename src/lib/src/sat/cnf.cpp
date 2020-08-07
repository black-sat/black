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

#include <black/logic/parser.hpp>

namespace black::internal 
{
  formula tseitin(formula f, std::vector<clause> &clauses);

  // TODO: disambiguate fresh variables
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    atom a = f.alphabet()->var(f);
    return a;
  }

  cnf to_cnf(formula f) {
    std::vector<clause> result;
    
    formula simple = tseitin(f, result);
    black_assert(simple.is<boolean>() || !has_constants(simple));

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

  formula tseitin(formula f, std::vector<clause> &clauses) {
    return f.match(
      [](boolean b) -> formula { return b; },
      [](atom a) -> formula { return a; },
      [&](conjunction, formula l, formula r) 
      {
        formula sl = tseitin(l, clauses);
        formula sr = tseitin(r, clauses);

        formula s = simplify(sl && sr);

        if(!s.is<boolean>()) {
          // clausal form for conjunctions:
          //   f <-> (l ∧ r) == (!f ∨ l) ∧ (!f ∨ r) ∧ (!l ∨ !r ∨ f)
          clauses.insert(end(clauses), {
            {{false, fresh(s)}, {true, fresh(sl)}},
            {{false, fresh(s)}, {true, fresh(sr)}},
            {{false, fresh(sl)}, {false, fresh(sr)}, {true, fresh(s)}}
          });
        }

        return s;
        
      },
      [&](disjunction, formula l, formula r) 
      {
        formula sl = tseitin(l, clauses);
        formula sr = tseitin(r, clauses);

        formula s = simplify(sl || sr);

        if(!s.is<boolean>()) {
          // clausal form for disjunctions:
          //   f <-> (l ∨ r) == (f ∨ !l) ∧ (f ∨ !r) ∧ (l ∨ r ∨ !f)
          clauses.insert(end(clauses), {
            {{true, fresh(s)}, {false, fresh(sl)}},
            {{true, fresh(s)}, {false, fresh(sr)}},
            {{true, fresh(sl)}, {true, fresh(sr)}, {false, fresh(s)}}
          });
        }

        return s;
      },
      [&](then, formula l, formula r) 
      {
        formula sl = tseitin(l, clauses);
        formula sr = tseitin(r, clauses);

        formula s = simplify(then(sl, sr));

        if(!s.is<boolean>()) {
          // clausal form for double implications:
          //    f <-> (l -> r) == (!f ∨ !l ∨ r) ∧ (f ∨ l) ∧ (f ∨ !r)
          clauses.insert(end(clauses), {
            {{false, fresh(s)}, {false, fresh(sl)}, {true, fresh(sr)}},
            {{true,  fresh(s)}, {true,  fresh(sl)}},
            {{true,  fresh(s)}, {false, fresh(sr)}}
          });
        }

        return s;        
      },
      [&](iff, formula l, formula r) 
      {
        formula sl = tseitin(l, clauses);
        formula sr = tseitin(r, clauses);

        formula s = simplify(iff(sl, sr));

        if(!s.is<boolean>()) {
          // clausal form for double implications:
          //    f <-> (l <-> r) == (!f ∨ !l ∨  r) ∧ (!f ∨ l ∨ !r) ∧
          //                       ( f ∨ !l ∨ !r) ∧ ( f ∨ l ∨  r)
          clauses.insert(end(clauses), {
            {{false, fresh(s)}, {false, fresh(sl)}, {true,  fresh(sr)}},
            {{false, fresh(s)}, {true,  fresh(sl)}, {false, fresh(sr)}},
            {{true,  fresh(s)}, {false, fresh(sl)}, {false, fresh(sr)}},
            {{true,  fresh(s)}, {true,  fresh(sl)}, {true,  fresh(sr)}}
          });
        }

        return s;
      },
      [&](negation, formula arg) {
        formula sarg = tseitin(arg, clauses);

        formula s = simplify(negation(sarg));

        if(!s.is<boolean>()) {
          // clausal form for negations:
          // f <-> !p == (!f ∨ !p) ∧ (f ∨ p)
          // TODO: handle NANDs, NORs, etc.. instead for a better translation
          clauses.insert(end(clauses), {
            {{false, fresh(s)}, {false, fresh(sarg)}},
            {{true,  fresh(s)}, {true,  fresh(sarg)}}
          });
        }

        return s;
      },
      [](otherwise) -> formula { black_unreachable(); }
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
