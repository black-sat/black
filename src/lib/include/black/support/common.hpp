//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#ifndef BLACK_COMMON_H
#define BLACK_COMMON_H

#include <black/support/assert.hpp>

#include <optional>
#include <type_traits>
#include <any>
#include <tuple>


// Include in the details namespace some commonly used type.
// Since they are not included in the main namespace, the declarations do not
// interefere with anything else.
namespace black::internal {

  using std::optional;
  using std::make_optional;
  using std::nullopt;

}

// Shorthand for perfect forwarding
#define FWD(a) std::forward<decltype(a)>(a)

// Shorthand for SFINAE-friendly small functions
#define RETURNS_DECLTYPE(Expr) \
  decltype(Expr) { return Expr; }

// The REQUIRES() macro, an easier to use wrapper around std::enable_if

// WARNING: this must stay on the same line.
#define REQUIRES(...) \
typename BLACK_REQUIRES_FRESH = void, typename std::enable_if<::black::internal::true_t<BLACK_REQUIRES_FRESH>::value && ::black::internal::all(__VA_ARGS__), int>::type = 0

#define REQUIRES_OUT_OF_LINE(...) \
typename BLACK_REQUIRES_FRESH, typename std::enable_if<::black::internal::true_t<BLACK_REQUIRES_FRESH>::value && ::black::internal::all(__VA_ARGS__), int>::type

#define BLACK_CONCAT(x, y) BLACK_CONCAT_2(x,y)
#define BLACK_CONCAT_2(x, y) x ## y

#define BLACK_REQUIRES_FRESH \
  BLACK_CONCAT(UNFULFILLED_TEMPLATE_REQUIREMENT_, __LINE__)

// Macro to require well-formedness of some dependent expression
#define WELL_FORMED(Expr) typename = std::void_t<decltype(Expr)>

namespace black::internal {

  // Simple utility to get the overloading of multiple lambdas (for example)
  // not used for formula::match but useful to work with std::visit
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  // Always true type trait used for SFINAE tricks
  template<typename T>
  struct true_t : std::true_type { };

  //
  // constexpr function all(...) that returns true if all its arguments are
  // either true booleans or have a true ::value member
  // 
  // Used in the implementation of REQUIRES macro above.
  //
  constexpr bool all() { return true; }

  template<typename ...Args>
  constexpr
  bool all(bool b, Args ...args)
  {
      return b && all(args...);
  }

  template<typename T, typename ...Args, bool Tb = T::value>
  constexpr
  bool all(T, Args ...args)
  {
      return Tb && all(args...);
  }

  template<typename T, typename ...Args>
  constexpr bool same_type() {
      return all(std::is_same<T, Args>()...);
  }

  //
  // Useful utilities to work with strongly-typed enums
  //
  template <typename E, REQUIRES(std::is_enum_v<E>)>
  constexpr auto to_underlying(E e) noexcept
  {
      return static_cast<std::underlying_type_t<E>>(e);
  }

  template <typename E, REQUIRES(std::is_integral_v<E>)>
  constexpr auto to_underlying(E e) noexcept
  {
      return static_cast<E>(e);
  }

  template<typename E, REQUIRES(std::is_enum_v<E>)>
  constexpr E from_underlying(std::underlying_type_t<E> v) noexcept {
    return static_cast<E>(v);
  }
} // namespace black::internal

//
// std::hash specialization for tuples and pairs.
// See https://stackoverflow.com/a/27952689/3206471
// and https://stackoverflow.com/questions/35985960
// for an explanation of the hashing function combination technique
//
namespace black::internal {
  inline size_t hash_combine(size_t lhs, size_t rhs) {
    static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);

    if constexpr(sizeof(size_t) == 4) {
      lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    } else if constexpr(sizeof(size_t) == 8) {
      lhs ^= rhs + 0x9e3779b97f4a7c15 + (lhs << 6) + (lhs >> 2);
    }

    return lhs;
  }

  template<typename T, typename ...Ts, size_t ...Idx>
  std::tuple<Ts...>
  tuple_tail_impl(std::tuple<T,Ts...> const&t, std::index_sequence<Idx...>) {
    return std::make_tuple(std::get<Idx + 1>(t)...);
  }

  template<typename T, typename ...Ts>
  std::tuple<Ts...> tuple_tail(std::tuple<T, Ts...> const&t) {
    return tuple_tail_impl(t, std::make_index_sequence<sizeof...(Ts)>{});
  }
}

// NB: Declaring things in namespace std is allowed for specialization of
// standard templates
namespace std
{
  template<typename T>
  struct hash<tuple<T>> {
    size_t operator()(tuple<T> const&t) const {
      return hash<T>{}(get<0>(t));
    }
  };

  template<typename T, typename ...Ts>
  struct hash<tuple<T,Ts...>> {
    size_t operator()(tuple<T,Ts...> const&t) const {
      using namespace ::black::internal;

      hash<T> h1;
      hash<tuple<Ts...>> h2;

      return hash_combine(h1(std::get<0>(t)), h2(tuple_tail(t)));
    }
  };

  template<typename S, typename T>
  struct hash<pair<S, T>>
  {
    size_t operator()(pair<S, T> const&v) const {
      return hash<tuple<S,T>>{}(make_tuple(v.first,v.second));
    }
  };
}

namespace black::internal
{
  //
  // Trait to detect whether a type is convertible to a tuple of some arbitrary
  // types
  //
  template<typename T>
  struct is_tuple_t : std::false_type {};

  template<typename ...Args>
  struct is_tuple_t<std::tuple<Args...>> : std::true_type {};

  template<typename T>
  constexpr bool is_tuple = is_tuple_t<std::decay_t<T>>::value;

  //
  // Check if a type is hashable
  // https://stackoverflow.com/questions/12753997
  //
  // TODO: check for proper enabled specializations
  //
  template <typename T, typename = void>
  struct is_hashable_t : std::false_type { };

  template <typename T>
  struct is_hashable_t<T,
    std::void_t<
      decltype(std::declval<std::hash<std::decay_t<T>>>()(std::declval<T>()))
    >
  > : std::true_type { };

  template <typename T>
  constexpr bool is_hashable = is_hashable_t<T>::value;

  //
  // Type-erased hashable value
  //
  class any_hashable
  {
  public:
    any_hashable() = default;
    any_hashable(any_hashable const&) = default;
    any_hashable(any_hashable&&) = default;

    template<
      typename T,
      REQUIRES(!std::is_convertible_v<T, any_hashable>),
      REQUIRES(is_hashable<std::decay_t<T>>)
    >
    explicit any_hashable(T&& value)
      : _any(FWD(value)), _hash(make_hasher(value)), _cmp(make_cmp(value)) {}

    size_t hash() const {
      black_assert(_any.has_value());
      return _hash(_any);
    }

    any_hashable &operator=(any_hashable const&) = default;
    any_hashable &operator=(any_hashable&&) = default;

    bool operator==(any_hashable const&other) const {
      return _cmp(_any, other);
    }

    template<typename T, REQUIRES(is_hashable<std::decay_t<T>>)>
    any_hashable &operator=(T&& value) {
      _any = FWD(value);
      _hash = make_hasher(value);
      _cmp = make_cmp(value);
      return *this;
    }

    template<typename T>
    bool is() const {
      return std::any_cast<T>(&_any) != nullptr;
    }

    template<typename T>
    optional<T> to() const & {
      if(T const*ptr = std::any_cast<T>(&_any); ptr)
        return optional<T>{*ptr};
      return nullopt;
    }

    template<typename T>
    optional<T> to() && {
      if(T const*ptr = std::any_cast<T>(&_any); ptr)
        return optional<T>{std::move(*ptr)};
      return nullopt;
    }

    template<typename T>
    T const* get() const & { return std::any_cast<T>(&_any); }

    template<typename T>
    T *get() & { return std::any_cast<T>(&_any); }

    std::any const&any() const { return _any; }

  private:
    using hasher_t = size_t (*)(std::any const&);
    using comparator_t = bool (*)(std::any const&, any_hashable const&);

    std::any _any;
    hasher_t _hash;
    comparator_t _cmp;

    template<typename T>
    hasher_t make_hasher(T const&) {
      return [](std::any const&me) -> size_t {
        T const *v = std::any_cast<T>(&me);
        black_assert(v != nullptr);

        return std::hash<T>{}(*v);
      };
    }

    template<typename T>
    comparator_t make_cmp(T const&) {
      return [](std::any const&me, any_hashable const&other) -> bool {
        T const* v = std::any_cast<T>(&me);
        T const* otherv = other.get<T>();

        black_assert(v != nullptr);

        return otherv != nullptr && *v == *otherv;
      };
    }
  };
}

// std::hash specialization for any_hashable
namespace std {
  template<>
  struct hash<black::internal::any_hashable> {
    size_t operator()(black::internal::any_hashable const&h) const {
      return h.hash();
    }
  };
}


#endif // BLACK_COMMON_H
