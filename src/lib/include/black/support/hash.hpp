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

namespace black::support::internal {

  //
  // Function to combine different hashe values into one.
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

  class hasher 
  {
  public:
    hasher() = default;
    hasher(hasher const&) = default;

    hasher &operator=(hasher const&) = default;

    size_t hash() const { return _hash; }

    template<hashable T>
    friend hasher operator+(hasher h, T&& v) {
      return hasher{
        hash_combine(
          h.hash(), 
          std::hash<std::remove_cvref_t<T>>{}(std::forward<T>(v))
        )
      };
    }

    template<std::ranges::range R>
      requires hashable<std::ranges::range_value_t<R>>
    friend hasher operator+(hasher h, R const&r) {
      for(auto v : r) {
        h += v;
      }

      return h;
    }

    template<typename T>
      requires requires (T v) { std::get<0>(v); }
    friend hasher operator+(hasher h, T const& t) {
      std::apply([&](auto ...v) {
        h = (h + ... + v);
      }, t);
      
      return h;
    }

    template<hashable T>
    friend hasher operator+(T&& v, hasher h) {
      return h + std::forward<T>(v);
    }

    template<std::ranges::range R>
      requires hashable<std::ranges::range_value_t<R>>
    friend hasher operator+(R const& r, hasher h) {
      return h + r;
    }

    template<typename T>
      requires requires (T v) { std::get<0>(v); }
    friend hasher operator+(T const& t, hasher h) {
      return h + t;
    }

    template<hashable T>
    hasher &operator+=(T&& v) {
      *this = *this + v;

      return *this;
    }

    template<std::ranges::range R>
      requires hashable<std::ranges::range_value_t<R>>
    hasher &operator+=(R const& r) {
      *this = *this + r;

      return *this;
    }

    template<typename T>
      requires requires (T v) { std::get<0>(v); }
    hasher &operator+=(T const& t) {
      *this = *this + t;
      
      return *this;
    }

  private:
    hasher(size_t h) : _hash{h} { }
    size_t _hash = 0;
  };

  template<typename ...Ts>
  size_t hash(Ts&& ...args) {
    return (hasher{} + ... + std::forward<Ts>(args)).hash();
  }

}

namespace black::support {
  using internal::hasher;
  using internal::hash;
  using internal::hashable;
}

#endif // BLACK_SUPPORT_HASH_HPP
