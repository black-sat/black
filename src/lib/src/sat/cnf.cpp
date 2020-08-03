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

namespace black::internal 
{
  void tseitin(formula f, std::vector<clause> &clauses);

  cnf to_cnf(formula f) {
    cnf result;
    tseitin(f, result.clauses);
    
    return result;
  }

  // TODO: disambiguate fresh variables
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    return f.alphabet()->var(f);
  }
  
  void tseitin(formula f,  std::vector<clause> &clauses) {
    f.match(
      [ ](atom) { /* nop */ },
      [&](conjunction, formula l, formula r) {
        // clausal form for conjunctions:
        //   f <-> (l ∧ r) == (!f ∨ l) ∧ (!f ∨ r) ∧ (!l ∨ !r ∨ f)
        clauses.insert(end(clauses), {
          {{false, fresh(f)}, {true, fresh(l)}},
          {{false, fresh(f)}, {true, fresh(r)}},
          {{false, fresh(l)}, {false, fresh(r)}, {true, fresh(f)}}
        });

        tseitin(l, clauses);
        tseitin(r, clauses);
      },
      [&](disjunction, formula l, formula r) {
        // clausal form for disjunctions:
        //   f <-> (l ∨ r) == (f ∨ !l) ∧ (f ∨ !r) ∧ (l ∨ r ∨ !f)
        clauses.insert(end(clauses), {
          {{true, fresh(f)}, {false, fresh(l)}},
          {{true, fresh(f)}, {false, fresh(r)}},
          {{true, fresh(l)}, {true, fresh(r)}, {false, fresh(f)}}
        });

        tseitin(l, clauses);
        tseitin(r, clauses);
      },
      [&](negation, formula arg) {
        // clausal form for negations:
        // f <-> !p == (!f ∨ !p) ∧ (f ∨ p)
        // TODO: handle NANDs, NORs, etc.. for a better translation
        clauses.insert(end(clauses), {
          {{false, fresh(f)}, {false, fresh(arg)}},
          {{true,  fresh(f)}, {true,  fresh(arg)}}
        });

        tseitin(arg, clauses);
      },
      [](otherwise) {
        black_unreachable();
      }
    );
  }
}
