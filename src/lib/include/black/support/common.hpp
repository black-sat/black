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

// Overload of callables, useful for ADT-like patterns
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// The REQUIRES() macro, an easier to use wrapper around std::enable_if

// WARNING: this must stay on the same line.
#define REQUIRES(...) \
typename BLACK_REQUIRES_FRESH = void, typename std::enable_if<::black::details::true_t<BLACK_REQUIRES_FRESH>::value && ::black::details::all(__VA_ARGS__), int>::type = 0

#define BLACK_CONCAT__(x, y) BLACK_CONCAT_2__(x,y)
#define BLACK_CONCAT_2__(x, y) x ## y

#define BLACK_REQUIRES_FRESH \
  BLACK_CONCAT__(UNFULFILLED_TEMPLATE_REQUIREMENT_, __LINE__)

namespace black::details {

  template<typename T>
  struct true_t : std::true_type { };

  template<typename T>
  using void_t = void;

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
} // namespace black::details

namespace black {

    using details::true_t;
    using details::void_t;

    using details::neg;
    using details::all;
    using details::any;

    using details::same_type;
}


#endif // BLACK_COMMON_H
