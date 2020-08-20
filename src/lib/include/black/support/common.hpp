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

#include <black/support/config.hpp>

#include <optional>
#include <type_traits>
#include <string_view>

// Include in the details namespace some commonly used type.
// Since they are not included in the main namespace, the declarations do not
// interefere with anything else.
namespace black::internal {

  using std::optional;
  using std::make_optional;
  using std::nullopt;

}

namespace black {
  inline std::string_view version = BLACK_VERSION;
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

#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#endif // BLACK_COMMON_H
