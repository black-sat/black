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


#ifndef BLACK_SUPPORT_HASH_HPP
#define BLACK_SUPPORT_HASH_HPP

#include <functional>
#include <ranges>
#include <tuple>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

namespace black::support::internal {

  //
  // Function to combine different hash values into one.
  // See https://stackoverflow.com/a/27952689/3206471
  // and https://stackoverflow.com/questions/35985960
  // for an explanation of the algorithm
  //
  inline size_t hash_combine(size_t lhs, size_t rhs) {
    static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);

    if constexpr(sizeof(size_t) == 4) {
      lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    } else if constexpr(sizeof(size_t) == 8) {
      lhs ^= rhs + 0x9e3779b97f4a7c15 + (lhs << 6) + (lhs >> 2);
    }

    return lhs;
  }

  template<typename T>
  concept hashable = requires(T t) {
    std::hash<std::remove_cvref_t<T>>{}(t);
  };

  template<typename T>
  size_t hash(T const& arg) {
    return std::hash<T>{}(arg);
  }

  template<std::ranges::range R>
    requires hashable<std::ranges::range_value_t<R>>
  size_t hash(R const& r) {
    size_t h = 0;
    for(auto elem : r)
      h = hash_combine(h, hash(elem));
    return h;
  }

  template<typename T>
    requires requires (T v) { std::get<0>(v); }
  size_t hash(T const& v) {
    return std::apply(
      [](auto arg, auto ...args) {
        size_t h = hash(arg);
        ((h = hash_combine(h, hash(args))), ...);
        return h;
      }, v);
  }

  template<typename Arg, typename ...Args>
  size_t hash(Arg const& arg, Args const& ...args) {
    size_t h = hash(arg);
    ((h = hash_combine(h, hash(args))), ...);
    return h;
  }

  //
  // Short aliases for hopscotch map/set
  //
  template<typename Key, typename Value>
  using map = tsl::hopscotch_map<Key, Value>;
  
  template<typename T>
  using set = tsl::hopscotch_set<T>;
}

namespace black::support {
  using internal::hash;
  using internal::hashable;
  using internal::map;
  using internal::set;
}

#endif // BLACK_SUPPORT_HASH_HPP
