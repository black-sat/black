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
  void tseitin(formula f, std::vector<clause> &clauses);

  // TODO: disambiguate fresh variables
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    atom a = f.alphabet()->var(f);
    return a;
  }

  bool has_constants(formula f)
  {
    return f.match(
      [](boolean) { return true; },
      [](atom) { return false; },
      [](unary, formula op) { return has_constants(op); },
      [](binary, formula l, formula r) { 
        return has_constants(l) || has_constants(r);
      }
    );
  }

  cnf to_cnf(formula f) {
    std::vector<clause> result;
    
    formula simple = simplify(f);
    if(auto b = simple.to<boolean>(); b) {
      if(b->value())
        return result;
      else {
        result.push_back({});
        return result;
      }
    }

    black_assert(!has_constants(simple));
    
    tseitin(simple, result);
    result.push_back({{true, fresh(simple)}});

    return {result};
  }
  
  formula simplify(formula f) {
    alphabet &sigma = *f.alphabet();
    return f.match(
      [ ](boolean b) -> formula { return b; },
      [ ](atom a) -> formula { return a; },
      [&](negation, formula op) -> formula {
        formula arg = simplify(op);
        std::optional<boolean> barg = arg.to<boolean>();
        if(!barg)
          return negation(arg);
        
        if(barg->value())
          return sigma.bottom();
        else
          return sigma.top();
      },
      [&](conjunction, formula l, formula r) -> formula {
        formula sl = simplify(l), sr = simplify(r);
        optional<boolean> bl = sl.to<boolean>(), br = sr.to<boolean>();

        if(!bl && !br)
          return conjunction(sl,sr);

        if(bl && !br) {
          return bl->value() ? sr : sigma.bottom();
        }
        
        if(!bl && br)
          return br->value() ? sl : sigma.bottom();

        return sigma.boolean(bl->value() && br->value());
      },
      [&](disjunction, formula l, formula r) -> formula {
        formula sl = simplify(l), sr = simplify(r);
        optional<boolean> bl = sl.to<boolean>(), br = sr.to<boolean>();

        if(!bl && !br)
          return disjunction(sl,sr);

        if(bl && !br)
          return bl->value() ? sigma.top() : sr;
        
        if(!bl && br)
          return br->value() ? sigma.top() : sl;
          
        return sigma.boolean(bl->value() || br->value());
      },
      [&](then, formula l, formula r) -> formula {
        formula sl = simplify(l), sr = simplify(r);
        optional<boolean> bl = sl.to<boolean>(), br = sr.to<boolean>();

        if(!bl && !br)
          return then(sl,sr);

        if(bl && !br) {
          return bl->value() ? sr : sigma.top();
        }
        
        if(!bl && br)
          return br->value() ? sigma.top() : sigma.bottom();

        return sigma.boolean(!bl->value() || br->value());
      },
      [&](iff, formula l, formula r) -> formula {
        formula sl = simplify(l), sr = simplify(r);
        optional<boolean> bl = sl.to<boolean>(), br = sr.to<boolean>();

        if(!bl && !br)
          return iff(sl,sr);

        if(bl && !br) {
          return bl->value() ? sr : !sr;
        }
        
        if(!bl && br)
          return br->value() ? sl : !sl;

        return sigma.boolean(bl->value() == br->value());
      },
      [](otherwise) -> formula { black_unreachable(); }
    );
  }

  void tseitin(formula f,  std::vector<clause> &clauses) {
    f.match(
      [&](boolean) {
        // 🤷‍♂️
        black_unreachable();
      },
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
      [&](then, formula l, formula r) {
        // clausal form for double implications:
        //    f <-> (l -> r) == (!f ∨ !l ∨ r) ∧ (f ∨ l) ∧ (f ∨ !r)
        clauses.insert(end(clauses), {
          {{false, fresh(f)}, {false, fresh(l)}, {true, fresh(r)}},
          {{true,  fresh(f)}, {true,  fresh(l)}},
          {{true,  fresh(f)}, {false, fresh(r)}}
        });

        tseitin(l, clauses);
        tseitin(r, clauses);
      },
      [&](iff, formula l, formula r) {
        // clausal form for double implications:
        //    f <-> (l <-> r) == (!f ∨ !l ∨  r) ∧ (!f ∨ l ∨ !r) ∧
        //                       ( f ∨ !l ∨ !r) ∧ ( f ∨ l ∨  r)
        clauses.insert(end(clauses), {
          {{false, fresh(f)}, {false, fresh(l)}, {true,  fresh(r)}},
          {{false, fresh(f)}, {true,  fresh(l)}, {false, fresh(r)}},
          {{true,  fresh(f)}, {false, fresh(l)}, {false, fresh(r)}},
          {{true,  fresh(f)}, {true,  fresh(l)}, {true,  fresh(r)}}
        });

        tseitin(l, clauses);
        tseitin(r, clauses);
      },
      [&](negation, formula arg) {
        // clausal form for negations:
        // f <-> !p == (!f ∨ !p) ∧ (f ∨ p)
        // TODO: handle NANDs, NORs, etc.. instead for a better translation
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
