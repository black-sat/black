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
#include <algorithm>
#include <memory>

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
  size_t hash(T const& arg)
  {
    return std::hash<T>{}(arg);
  }

  template<typename T>
    requires requires(T v) { v.hash(); }
  size_t hash(T const& v) {
    return v.hash();
  }

  // The following functions are declared before being defined in order to 
  // allow each of them to find each other by unqualified lookup
  template<std::ranges::range R>
  size_t hash(R const& r);

  template<typename Arg, typename ...Args>
  size_t hash(std::tuple<Arg, Args...> const& t);

  size_t hash(std::tuple<>);

  // Implementations
  template<std::ranges::range R>
  size_t hash(R const& r) {
    size_t h = 0;
    for(auto elem : r)
      h = hash_combine(h, hash(elem));
    return h;
  }

  template<typename Arg, typename ...Args>
  size_t hash(std::tuple<Arg, Args...> const& t) {
    return std::apply(
      [](auto arg, auto ...args) {
        size_t h = hash(arg);
        ((h = hash_combine(h, hash(args))), ...);
        return h;
      }, t);
  }

  inline size_t hash(std::tuple<>) {
    return 0;
  }

  // Variadic version
  template<typename Arg, typename ...Args>
    requires (sizeof...(Args) > 0)
  size_t hash(Arg const& arg, Args const& ...args) {
    size_t h = hash(arg);
    ((h = hash_combine(h, hash(args))), ...);
    return h;
  }
}

template<typename T>
  requires requires(T v) { v.hash(); }
struct std::hash<T> {
  size_t operator()(T const& v) const {
    return v.hash();
  }
};

namespace black::support {
  using internal::hash;
}

#endif // BLACK_SUPPORT_HASH_HPP
