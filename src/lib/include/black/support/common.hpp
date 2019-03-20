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

#include <debug_assert.hpp>

#include <optional>
#include <type_traits>


// Include in the details namespace some commonly used type.
// Since they are not included in the main namespace, the declarations do not
// interefere with anything else.
namespace black::details {

  using std::optional;
  using std::make_optional;
  using std::nullopt;

}

// Custom assert and unreachable assertion macros built on top of debug_assert
// library.
namespace black::details {
  // Settings for the customization of foonathan's DEBUG_ASSERT macro
  struct black_assert_t
    : debug_assert::default_handler,
      debug_assert::set_level<1> {};

} // black::details

#define black_assert(Expr) \
  DEBUG_ASSERT(Expr, ::black::details::black_assert_t{})
#define black_unreachable() \
  DEBUG_UNREACHABLE(::black::details::black_assert_t{})

// Tool function to assert the validity of a pointer in an expression
template<typename T>
constexpr T *non_null(T *p) noexcept {
  black_assert(p != nullptr);
  return p;
}

namespace black::details {
  // First-match-first-called apply function, used in formula matchers
  template<typename ...Args, typename F>
  auto apply_first(std::tuple<Args...> args, F f)
    //-> decltype(std::apply(f, args))
  {
    return std::apply(f, args);
  }

  template<typename ...Args, typename F, typename ...Fs>
  auto apply_first(std::tuple<Args...> args, F f, Fs ...fs)
    // -> std::conditional_t<std::is_invocable_v<F, Args...>,
    //      std::invoke_result_t<F, Args...>,
    //      decltype(apply_first(args, fs...))>
  {
    if constexpr(std::is_invocable_v<F, Args...>) {
      return std::apply(f, args);
    } else
      return apply_first(args, fs...);
  }
}

// Shorthand for perfect forwarding
#define FWD(a) std::forward<decltype(a)>(a)

// The REQUIRES() macro, an easier to use wrapper around std::enable_if

// WARNING: this must stay on the same line.
#define REQUIRES(...) \
typename BLACK_REQUIRES_FRESH = void, typename std::enable_if<::black::details::true_t<BLACK_REQUIRES_FRESH>::value && ::black::details::all(__VA_ARGS__), int>::type = 0

#define REQUIRES_OUT_OF_LINE(...) \
typename BLACK_REQUIRES_FRESH, typename std::enable_if<::black::details::true_t<BLACK_REQUIRES_FRESH>::value && ::black::details::all(__VA_ARGS__), int>::type

#define BLACK_CONCAT(x, y) BLACK_CONCAT_2(x,y)
#define BLACK_CONCAT_2(x, y) x ## y

#define BLACK_REQUIRES_FRESH \
  BLACK_CONCAT(UNFULFILLED_TEMPLATE_REQUIREMENT_, __LINE__)

// Macro to require well-formedness of some dependent expression
#define WELL_FORMED(Expr) typename = std::void_t<decltype(Expr)>

namespace black::details {

  template<typename T>
  struct true_t : std::true_type { };

  template<typename T, bool Tb = T::value>
  constexpr bool metapredicate() {
      return Tb;
  }

  template<bool B>
  constexpr bool metapredicate() {
      return B;
  }

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

  constexpr bool neg(bool b) { return not b; }

  template<typename T, bool Tb = T::value>
  constexpr bool neg(T) { return not Tb; }

  template<typename ...Args>
  constexpr bool any(Args ...args) {
      return not all(neg(args)...);
  }

  //
  // Check if a type is hashable
  // https://stackoverflow.com/questions/12753997
  //
  template <typename T, typename = void>
  struct is_hashable_t : std::false_type { };

  template <typename T>
  struct is_hashable_t<T,
    std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>>
    : std::true_type { };

  template <typename T>
  constexpr bool is_hashable = is_hashable_t<T>::value;

  //
  // Useful utilities to work with strongly-typed enums
  //
  template <typename E, REQUIRES(std::is_enum_v<E>)>
  constexpr auto to_underlying(E e) noexcept
  {
      return static_cast<std::underlying_type_t<E>>(e);
  }

  template<typename E, REQUIRES(std::is_enum_v<E>)>
  constexpr E from_underlying(std::underlying_type_t<E> v) noexcept {
    return static_cast<E>(v);
  }

  // Utility to check equality of a variable number of parameters
  // Useful for parameter packs
  bool constexpr all_equal() { return true; }

  template<typename Arg, typename ...Args>
  bool constexpr all_equal(Arg &&arg, Args&& ...args) {
    return ((std::forward<Arg>(arg) == std::forward<Args>(args)) && ...);
  }
} // namespace black::details

namespace black {
    using details::true_t;

    using details::neg;
    using details::all;
    using details::any;

    using details::same_type;
}


#endif // BLACK_COMMON_H