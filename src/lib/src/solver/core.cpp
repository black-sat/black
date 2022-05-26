//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#include <black/solver/core.hpp>

#include <black/logic/formula.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/solver/solver.hpp>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

#include <fmt/format.h>

namespace black::internal {

  struct K_data_t {
    size_t size;
    size_t n;
  };

  static size_t K_impl(formula f, tsl::hopscotch_map<formula, K_data_t> &ks) {
    return f.match(
      [](boolean) { return 1; },
      [](proposition) { return 1; },
      [&](unary, formula arg) {
        K_data_t data = ks.contains(f) ? ks[f] : K_data_t{0, 0};
        if(data.size == 0)
          data.size = 1 + K_impl(arg, ks);
        data.n += 1;
        ks[f] = data;

        return data.size;
      },
      [&](binary, formula left, formula right) {
        K_data_t data = ks.contains(f) ? ks[f] : K_data_t{0, 0};
        if(data.size == 0)
          data.size = 1 + K_impl(left, ks) + K_impl(right, ks);
        data.n += 1;
        ks[f] = data;

        return data.size;
      },
      [](first_order) -> size_t { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  tsl::hopscotch_map<formula, K_data_t> compute_K(formula f) {
    tsl::hopscotch_map<formula, K_data_t> ks;
    K_impl(f, ks);
    return ks;
  }

  std::vector<std::vector<formula>>
  group_by_K(formula f, tsl::hopscotch_map<formula, K_data_t> ks) {
    std::vector<std::vector<formula>> groups(ks[f].size + 1);

    for(auto [subf, k] : ks) {
      size_t K = k.size * k.n;
      black_assert(K < groups.size());
      groups[K].push_back(subf);
    }

    return groups;
  }

  static size_t increment(std::vector<bool> &v) {
    size_t pos = 0;
    
    while(pos < v.size() && v[pos]) {
      v[pos] = false;
      ++pos;
    }
    if(pos < v.size())
      v[pos] = true;

    return pos;
  }

  static 
  formula replace(formula f, tsl::hopscotch_set<formula> const& dontcares) {
    using namespace std::literals;

    if(dontcares.contains(f))
      return f.sigma()->prop(core_placeholder_t{f});
    
    return f.match(
      [&](boolean) { return f; },
      [&](proposition) { return f; },
      [&](unary u, formula arg) {
        return unary(u.formula_type(), replace(arg, dontcares));
      },
      [&](binary b, formula left, formula right) {
        return binary(
          b.formula_type(), 
          replace(left, dontcares), replace(right, dontcares)
        );
      },
      [](first_order) -> formula { black_unreachable(); }
    );
  }

  formula unsat_core(formula f) {
    auto ks = compute_K(f);
    auto groups = group_by_K(f, ks);

    size_t k = groups.size() - 1;
    for(auto it = groups.rbegin(); it != groups.rend(); ++it, --k) {
      auto &group = *it;
      
      std::vector<bool> v(group.size());
      
      while(increment(v) != v.size()) {
        tsl::hopscotch_set<formula> dontcares;
        for(size_t i = 0; i < v.size(); ++i) {
          if(v[i])
            dontcares.insert(group[i]);
        }
       
        formula candidate = replace(f, dontcares);

        solver slv;
        slv.set_formula(candidate);
        tribool res = slv.solve();
        if(res == false)
          return unsat_core(candidate);
      }      
    }
    return f;
  }

}