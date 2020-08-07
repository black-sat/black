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

#include <black/logic/formula.hpp>
#include <black/logic/alphabet.hpp>

#include <optional>

namespace black::internal {

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

  formula simplify_deep(formula f) {
    return f.match(
      [](boolean b) { return simplify(b); },
      [](atom a) { return simplify(a); },
      [](unary u, formula op) { 
        return simplify(unary(u.formula_type(), simplify_deep(op)));
      },
      [](binary b, formula l, formula r) { 
        return 
          simplify(
            binary(b.formula_type(), simplify_deep(l), simplify_deep(r))
          );
      }
    );
  }

  formula simplify(formula f) {
    alphabet &sigma = *f.alphabet();
    return f.match(
      [ ](boolean b) -> formula { return b; },
      [ ](atom a) -> formula { return a; },
      [&](negation n, formula op) -> formula {
        if(auto b = op.to<boolean>(); b)
          return sigma.boolean(!b->value());
        
        return n;
      },
      [&](conjunction c, formula l, formula r) -> formula {
        optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

        if(!bl && !br)
          return c;

        if(bl && !br) {
          return bl->value() ? r : sigma.bottom();
        }
        
        if(!bl && br)
          return br->value() ? l : sigma.bottom();

        return sigma.boolean(bl->value() && br->value());
      },
      [&](disjunction d, formula l, formula r) -> formula {
        optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

        if(!bl && !br)
          return d;

        if(bl && !br)
          return bl->value() ? sigma.top() : r;
        
        if(!bl && br)
          return br->value() ? sigma.top() : l;
          
        return sigma.boolean(bl->value() || br->value());
      },
      [&](then t, formula l, formula r) -> formula {
        optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

        if(!bl && !br)
          return t;

        if(bl && !br)
          return bl->value() ? r : sigma.top();
        
        if(!bl && br)
          return br->value() ? sigma.top() : sigma.bottom();

        return sigma.boolean(!bl->value() || br->value());
      },
      [&](iff ff, formula l, formula r) -> formula {
        optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

        if(!bl && !br)
          return ff;

        if(bl && !br)
          return bl->value() ? r : !r;
        
        if(!bl && br)
          return br->value() ? l : !l;

        return sigma.boolean(bl->value() == br->value());
      },
      [&](otherwise) -> formula { return f; /* TODO */ }
    );
  }
}
