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

#include <black/solver/hs.hpp>
#include <black/logic/prettyprint.hpp>

#include <tsl/hopscotch_set.h>

namespace black_internal::hs {

  using namespace black::logic::fragments::HS;

  static
  relation abstract(interval_op op) {
    return op.sigma()->relation(op);
  }
  
  static
  relation abstract(proposition prop) {
    return prop.sigma()->relation(prop);
  }

  static 
  logic::formula<logic::FO> abstract(formula f, variable x, variable y) {
    alphabet &sigma = *f.sigma();

    return f.match(
      [](boolean b) { return b; },
      [&](proposition p) {        
        return abstract(p)(x, y);
      },
      [&](negation, auto arg) { return !abstract(arg, x, y); },
      [&](conjunction c) { 
        return logic::big_and(sigma, c.operands(), [&](auto op) {
          return abstract(op, x, y);
        });
      },
      [&](disjunction d) { 
        return logic::big_or(sigma, d.operands(), [&](auto op) {
          return abstract(op, x, y);
        });
      },
      [&](implication, auto left, auto right) { 
        return logic::implies(abstract(left, x, y), abstract(right, x, y));
      },
      [&](iff, auto left, auto right) { 
        return logic::iff(abstract(left, x, y), abstract(right, x, y));
      },
      [&](interval_op op) {
        return abstract(op)(x, y);
      } 
    );
  }

  static
  void collect_ops(formula f, tsl::hopscotch_set<interval_op> &result) {
    if(auto op = f.to<interval_op>())
      result.insert(*op);

    for_each_child(f, [&](auto child) {
      collect_ops(child, result);
    });
  }

  static
  std::vector<interval_op> collect_ops(formula f) {
    tsl::hopscotch_set<interval_op> result;

    collect_ops(f, result);

    return std::vector<interval_op>(begin(result), end(result));
  }

  static
  void collect_props(formula f, tsl::hopscotch_set<proposition> &result) {
    if(auto prop = f.to<proposition>())
      result.insert(*prop);

    for_each_child(f, [&](auto child) {
      collect_props(child, result);
    });
  }

  static
  std::vector<proposition> collect_props(formula f) {
    tsl::hopscotch_set<proposition> result;

    collect_props(f, result);

    return std::vector<proposition>(begin(result), end(result));
  }

  static logic::formula<logic::FO> beta(variable x, variable y) {
    alphabet &sigma = *x.sigma();

    auto m = sigma.variable("min");
    auto M = sigma.variable("Max");

    return m <= x && x < y && y <= M;
  }

  static logic::formula<logic::FO> gamma(interval_op op) {
    alphabet &sigma = *op.sigma();

    auto x = sigma.variable("x");
    auto y = sigma.variable("y");
    auto w = sigma.variable("w");
    auto z = sigma.variable("z");

    return beta(x, y) && beta(w, z) && op.match(
      [&](after)        { return y == w; },
      [&](before)       { return x == z; },
      [&](later)        { return y < w; },
      [&](sooner)       { return x < z; },
      [&](overlaps)     { return x < w && w < y && y < z; },
      [&](overlappedby) { return y < z && z < x && x < w; },
      [&](begins)       { return x == w && z < y; },
      [&](beganby)      { return y == z && w < x; },
      [&](during)       { return x < w && z < y; },
      [&](contains)     { return y < z && w < x; },
      [&](ends)         { return y == z && x < w; },
      [&](endedby)      { return x == w && y < z; }
    );
  }

  logic::formula<logic::LTLFO> encode(logic::scope &xi, formula f) 
  {
    alphabet &sigma = *f.sigma();
    //auto e = sigma.proposition("e");

    auto ops = collect_ops(f);
    auto props = collect_props(f);

    auto Int = sigma.integer_sort();

    for(auto op : ops) {
      auto pred = sigma.relation(op);
      if(op.existential())
        xi.declare(pred, {Int, Int}, scope::non_rigid);
      else
        xi.declare(pred, {Int, Int}, scope::rigid);
    }

    for(auto prop : props) {
      auto pred = sigma.relation(prop);
      xi.declare(pred, {Int, Int}, scope::rigid);
    }

    auto m = sigma.variable("min");
    auto M = sigma.variable("Max");
    auto a = sigma.variable("a");
    auto b = sigma.variable("b");

    xi.declare(m, Int, scope::rigid);
    xi.declare(M, Int, scope::rigid);
    xi.declare(a, Int, scope::rigid);
    xi.declare(b, Int, scope::rigid);

    auto x = sigma.variable("x");
    auto y = sigma.variable("y");
    auto w = sigma.variable("w");
    auto z = sigma.variable("z");

    using ltlfo = logic::formula<logic::LTLFO>;

    auto unravel = big_and(sigma, ops, [&](auto op) -> ltlfo {
      if(op.existential())
        return logic::forall(
          {x[Int], y[Int]},
          logic::implies(
            beta(x, y) && abstract(op)(x, y),
            X(
              logic::exists({w[Int], z[Int]}, 
                gamma(op) && abstract(op.argument(), w, z)
              )
            )
          )
        );
      else
        return logic::forall(
          {x[Int], y[Int]},
          logic::implies(
            beta(x, y) && abstract(op)(x, y),
            wX(
              logic::forall({w[Int], z[Int]},
                logic::implies(gamma(op), abstract(op.argument(), w, z))
              )
            )
          )
        );
    });

    return beta(a, b) && abstract(f, a, b) && G(unravel);
  }

}
