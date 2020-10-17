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
    return f.match(
      [ ](boolean b) -> formula { return b; },
      [ ](atom a) -> formula { return a; },
      simplify_negation,
      simplify_and,
      simplify_or,
      simplify_implication,
      simplify_iff,
      simplify_tomorrow,
      simplify_eventually,
      simplify_always,
      simplify_until,
      simplify_release,
      [&](past) -> formula { /* TODO */ black_unreachable(); }
    );
  }

  formula simplify_negation(negation n, formula op) 
  {
    // !true -> false, !false -> true
    if(auto b = op.to<boolean>(); b)
      return n.alphabet()->boolean(!b->value()); 
    
    // !!p -> p
    if(auto nop = op.to<negation>(); nop)
      return simplify(nop->operand());

    return n;
  }

  formula simplify_and(conjunction c, formula l, formula r) {
    alphabet &sigma = *c.alphabet();
    optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return c;

    if(bl && !br) {
      return bl->value() ? r : sigma.bottom();
    }
    
    if(!bl && br)
      return br->value() ? l : sigma.bottom();

    return sigma.boolean(bl->value() && br->value());
  }

  formula simplify_or(disjunction d, formula l, formula r) {
    alphabet &sigma = *d.alphabet();
    optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return d;

    if(bl && !br)
      return bl->value() ? sigma.top() : r;
    
    if(!bl && br)
      return br->value() ? sigma.top() : l;
      
    return sigma.boolean(bl->value() || br->value());
  }

  formula simplify_implication(implication t, formula l, formula r) {
    alphabet &sigma = *t.alphabet();
    optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return t;

    if(bl && !br)
      return bl->value() ? r : sigma.top();
    
    if(!bl && br)
      return br->value() ? formula{sigma.top()} : formula{!l};

    return sigma.boolean(!bl->value() || br->value());
  }

  formula simplify_iff(iff f, formula l, formula r) {
    alphabet &sigma = *f.alphabet();
    optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return f;

    if(bl && !br)
      return bl->value() ? r : !r;
    
    if(!bl && br)
      return br->value() ? l : !l;

    return sigma.boolean(bl->value() == br->value());
  }

  formula simplify_tomorrow(tomorrow x, formula op) { 
    if(op.is<boolean>())
      return op;

    return x;
  }

  formula simplify_eventually(eventually e, formula op) { 
    if(op.is<boolean>())
      return op;

    return e;
  }

  formula simplify_always(always g, formula op) { 
    if(op.is<boolean>())
      return op;

    return g;
  }

  formula simplify_until(until u, formula l, formula r) {
    optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return u;

    if(bl && !br)
      return bl->value() ? F(r) : r;
    
    return *br;
  }

  formula simplify_release(release s, formula l, formula r) {
    optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return s;

    if(bl && !br)
      return bl->value() ? r : G(r);

    return *br;
  }
}
