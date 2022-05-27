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

namespace black::internal {

  struct K_data_t {
    size_t size;
    size_t n;
  };

  static size_t traverse_impl(
    formula f, tsl::hopscotch_map<formula, K_data_t> &ks, 
    size_t &next_placeholder
  ) {
    return f.match(
      [](boolean) { return 1; },
      [&](proposition p) {
        if(auto l = p.label<core_placeholder_t>(); l.has_value()) {
          if(l->n >= next_placeholder)
            next_placeholder = l->n + 1;
        }
        return 1;
      },
      [&](unary, formula arg) {
        K_data_t data = ks.contains(f) ? ks[f] : K_data_t{0, 0};
        if(data.size == 0)
          data.size = 1 + traverse_impl(arg, ks, next_placeholder);
        data.n += 1;
        ks[f] = data;

        return data.size;
      },
      [&](binary, formula left, formula right) {
        K_data_t data = ks.contains(f) ? ks[f] : K_data_t{0, 0};
        if(data.size == 0)
          data.size = 1 + traverse_impl(left, ks, next_placeholder) 
                        + traverse_impl(right, ks, next_placeholder);
        data.n += 1;
        ks[f] = data;

        return data.size;
      },
      [](first_order) -> size_t { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  std::pair<
    tsl::hopscotch_map<formula, K_data_t>,
    size_t
  >
  traverse(formula f) {
    tsl::hopscotch_map<formula, K_data_t> ks;
    size_t next_placeholder = 0;
    
    traverse_impl(f, ks, next_placeholder);
    
    return {ks, next_placeholder};
  }

  std::vector<std::vector<formula>>
  group_by_K(formula f, tsl::hopscotch_map<formula, K_data_t> ks) {
    std::vector<std::vector<formula>> groups(ks[f].size + 1);

    for(auto [subf, k] : ks) {
      size_t K = k.size * k.n;
      black_assert(K < groups.size());
      groups[K].push_back(subf);
    }

    std::reverse(begin(groups), end(groups));

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

  static std::vector<std::vector<bool>> build_bitsets(size_t size) {
    std::vector<std::vector<bool>> result;
    
    std::vector<bool> v(size);
    while(increment(v) != size) {
      result.push_back(v);
    }

    std::sort(begin(result), end(result), [](auto const& a, auto const& b) {
      size_t m = std::count_if(begin(a), end(a), [](bool a) { return a; });
      size_t n = std::count_if(begin(b), end(b), [](bool b) { return b; });

      return m > n;
    });

    return result;
  }

  static 
  formula replace_impl(
    formula f, tsl::hopscotch_set<formula> const& dontcares,
    tsl::hopscotch_map<formula, size_t> &indexes,
    size_t &next_index
  ) {
    using namespace std::literals;

    if(dontcares.contains(f)) {
      size_t index = indexes.contains(f) ? indexes[f] : next_index++;
      indexes.insert_or_assign(f, index);

      return f.sigma()->prop(core_placeholder_t{index});
    }
    
    return f.match(
      [&](boolean) { return f; },
      [&](proposition) { return f; },
      [&](unary u, formula arg) {
        return unary(
          u.formula_type(), 
          replace_impl(arg, dontcares, indexes, next_index)
        );
      },
      [&](binary b, formula left, formula right) {
        return binary(
          b.formula_type(), 
          replace_impl(left, dontcares, indexes, next_index), 
          replace_impl(right, dontcares, indexes, next_index)
        );
      },
      [](first_order) -> formula { black_unreachable(); }
    );
  }

  static
  tsl::hopscotch_set<formula> build_subset(
    std::vector<formula> const& universe, std::vector<bool> const&bitset
  ) {
    black_assert(universe.size() == bitset.size());
    tsl::hopscotch_set<formula> subset;
    for(size_t i = 0; i < bitset.size(); ++i) {
      if(bitset[i])
        subset.insert(universe[i]);
    }

    return subset;
  }

  static 
  formula replace(
    formula f, tsl::hopscotch_set<formula> const&dontcares,
    size_t next_index
  ) {
    tsl::hopscotch_map<formula, size_t> indexes;

    return replace_impl(f, dontcares, indexes, next_index);
  }

  formula unsat_core(formula f) {
    auto [ks, next_placeholder] = traverse(f);
    auto groups = group_by_K(f, ks);

    for(auto &group : groups) {
      auto bitsets = build_bitsets(group.size());

      for(auto &bitset : bitsets) 
      { 
        auto subset = build_subset(group, bitset);
        formula candidate = replace(f, subset, next_placeholder);

        solver slv;
        slv.set_formula(candidate);
        
        if(slv.solve() == false)
          return unsat_core(candidate);
      }      
    }
    return f;
  }

}