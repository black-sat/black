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

#include <black/logic//formula.hpp>
#include <black/logic//alphabet.hpp>

#include <optional>

namespace black::internal {

  bool has_constants(formula f)
  {
    return f.match(
      [](boolean) { return true; },
      [](proposition) { return false; },
      [](atom) { return false; },
      [](quantifier) { return false; },
      [](unary, formula op) { return has_constants(op); },
      [](binary, formula l, formula r) { 
        return has_constants(l) || has_constants(r);
      }
    );
  }

  formula simplify_deep(formula f) {
    return f.match( // LCOV_EXCL_LINE
      [](boolean b) { return simplify(b); },
      [](proposition p) { return simplify(p); },
      [](atom a) { return simplify(a); },
      [](quantifier q) { 
        return simplify(quantifier(
          q.quantifier_type(), q.var(), simplify_deep(q.matrix())
        ));
      },
      [](unary u, formula op) { 
        return simplify(unary(u.formula_type(), simplify_deep(op)));
      },
      [](binary b, formula l, formula r) { 
        return 
          simplify( // LCOV_EXCL_LINE
            binary(b.formula_type(), simplify_deep(l), simplify_deep(r))
          );
      }
    );
  }

  static
  formula simplify_negation(negation n, formula op) 
  {
    // !true -> false, !false -> true
    if(auto b = op.to<boolean>(); b)
      return n.sigma()->boolean(!b->value()); 
    
    // !!p -> p
    if(auto nop = op.to<negation>(); nop)
      return nop->operand();

    return n;
  }

  static
  formula simplify_and(conjunction c, formula l, formula r) {
    alphabet &sigma = *c.sigma();
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return c;

    if(bl && !br) {
      return bl->value() ? r : sigma.bottom();
    }
    
    if(!bl && br)
      return br->value() ? l : sigma.bottom();

    return sigma.boolean(bl->value() && br->value());
  }

  static
  formula simplify_or(disjunction d, formula l, formula r) {
    alphabet &sigma = *d.sigma();
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return d;

    if(bl && !br)
      return bl->value() ? sigma.top() : r;
    
    if(!bl && br)
      return br->value() ? sigma.top() : l;
      
    return sigma.boolean(bl->value() || br->value());
  }

  static
  formula simplify_implication(implication t, formula l, formula r) {
    alphabet &sigma = *t.sigma();
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return t;

    if(bl && !br)
      return bl->value() ? r : sigma.top();
    
    if(!bl && br)
      return br->value() ? formula{sigma.top()} : simplify(!l);

    return sigma.boolean(!bl->value() || br->value());
  }

  static
  formula simplify_iff(iff f, formula l, formula r) {
    alphabet &sigma = *f.sigma();
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return f;

    if(bl && !br)
      return bl->value() ? r : simplify(!r);
    
    if(!bl && br)
      return br->value() ? l : simplify(!l);

    return sigma.boolean(bl->value() == br->value());
  }

  static
  formula simplify_tomorrow(tomorrow x, formula op) { 
    if(op.is<boolean>())
      return op;

    return x;
  }

  static
  formula simplify_w_tomorrow(w_tomorrow x, formula op) {
    return op.match( // LCOV_EXCL_LINE
      [&](boolean b) -> formula {
        if(b.value())
          return b;
        return x;
      },
      [&](otherwise) { return x; }
    );
  }

  static
  formula simplify_eventually(eventually e, formula op) { 
    if(op.is<boolean>())
      return op;

    return e;
  }

  static
  formula simplify_always(always g, formula op) { 
    if(op.is<boolean>())
      return op;

    return g;
  }

  static
  formula simplify_until(until u, formula l, formula r) {
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return u;

    if(bl && !br)
      return bl->value() ? simplify(F(r)) : r;
    
    return *br;
  }

  static
  formula simplify_w_until(w_until w, formula l, formula r) {
    alphabet &sigma = *w.sigma();
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return w;

    if(bl && !br)
      return bl->value() ? sigma.top() : r;

    if(!bl && br)
      return br->value() ? formula{sigma.top()} : simplify(G(l));

    return sigma.boolean(bl->value() || br->value());
  }

  static
  formula simplify_release(release s, formula l, formula r) {
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return s;

    if(bl && !br)
      return bl->value() ? r : simplify(G(r));

    return *br;
  }

  static
  formula simplify_s_release(s_release s, formula l, formula r) {
    alphabet &sigma = *s.sigma();
    std::optional<boolean> bl = l.to<boolean>(), br = r.to<boolean>();

    if(!bl && !br)
      return s;
    
    if(bl && !br)
      return bl->value() ? r : sigma.bottom();

    if(!bl && br)
      return br->value() ? simplify(F(l)) : formula{sigma.bottom()};

    return sigma.boolean(bl->value() && br->value());
  }

  formula simplify(formula f) {
    return f.match( // LCOV_EXCL_LINE
      [ ](boolean b) -> formula { return b; },
      [ ](proposition p) -> formula { return p; },
      [ ](atom a) -> formula { return a; },
      [ ](quantifier q) -> formula { return q; },
      simplify_negation,
      simplify_and,
      simplify_or,
      simplify_implication,
      simplify_iff,
      simplify_tomorrow,
      simplify_w_tomorrow,
      simplify_eventually,
      simplify_always,
      simplify_until,
      simplify_release,
      simplify_w_until,
      simplify_s_release,
      [&](past) -> formula { /* TODO */ black_unreachable(); } // LCOV_EXCL_LINE
    );
  }
}
