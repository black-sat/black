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

#include <type_traits>
#include <string_view>
#include <variant>
#include <tuple>

#ifdef _MSC_VER
  #define BLACK_EXPORT __declspec(dllexport)
#else
  #define BLACK_EXPORT
#endif

#if defined(_MSC_VER)
  #include <BaseTsd.h>
  using ssize_t = SSIZE_T;
#endif

namespace black::support::internal 
{
  // Simple utility to get the overloading of multiple lambdas (for example)
  // not used for formula::match but useful to work with std::visit
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  //
  // Useful utilities to work with strongly-typed enums
  //
  // GCOV false negatives
  template <typename E>
    requires std::is_enum_v<E>
  constexpr auto to_underlying(E e) noexcept // LCOV_EXCL_LINE
  {
      return static_cast<std::underlying_type_t<E>>(e); // LCOV_EXCL_LINE
  }

  template<typename E>
    requires std::is_enum_v<E>
  constexpr E from_underlying(std::underlying_type_t<E> v) noexcept {
    return static_cast<E>(v);
  }

  //
  // Misc metaprogramming utilities
  //

  template<typename T, typename U>
  struct tuple_cons;

  template<typename T, typename ...Us>
  struct tuple_cons<T, std::tuple<Us...>> {
    using type = std::tuple<T, Us...>;
  };

  template<typename T, typename ...Us>
  using tuple_cons_t = typename tuple_cons<T, Us...>::type;

  template<typename T, typename U>
  struct tuple_contains : std::false_type { };

  template<typename ...Ts, typename U>
  struct tuple_contains<std::tuple<Ts...>, U> : 
    std::disjunction<std::is_same<Ts, U>...> { };

  template<typename T, typename U>
  inline constexpr bool tuple_contains_v = tuple_contains<T, U>::value;

  template<typename T, typename ...Args>
    requires (std::is_same_v<T, Args> || ...)
  bool variant_is(std::variant<Args...> const& v) {
    return std::holds_alternative<T>(v);
  }

  template<typename T, typename ...Args>
  bool variant_is(std::variant<Args...> const&) {
    return false;
  }

  template<typename T, typename ...Args>
    requires (std::is_same_v<T, Args> || ...)
  std::optional<T> variant_get(std::variant<Args...> const& v) {
    return std::get<T>(v);
  }

  template<typename T, typename ...Args>
  std::optional<T> variant_get(std::variant<Args...> const&) {
    return {};
  }

  //
  // Thanks to Leonardo Taglialegne
  //
  inline std::pair<int, int> double_to_fraction(double n) {
    uint64_t a = (uint64_t)floor(n), b = 1;
    uint64_t c = (uint64_t)ceil(n), d = 1;

    uint64_t num = 1;
    uint64_t denum = 1;
    while(
      a + c <= (uint64_t)std::numeric_limits<int>::max() &&
      b + d <= (uint64_t)std::numeric_limits<int>::max() &&
      ((double)num/(double)denum != n)
    ) {
      num = a + c;
      denum = b + d;

      if((double)num/(double)denum > n) {
        c = num;
        d = denum;
      } else {
        a = num;
        b = denum;
      }
    }

    return {static_cast<int>(num), static_cast<int>(denum)};
  }
  
  inline constexpr std::string_view license =
R"(
BLACK - Bounded Lᴛʟ sAtisfiability ChecKer

(C) 2019-2023 Nicola Gigante
    2019-2021 Luca Geatti
    2020      Gabriele Venturato

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
)";
}

namespace black::support {
  using internal::overloaded;
  using internal::to_underlying;
  using internal::from_underlying;
  using internal::tuple_cons;
  using internal::tuple_cons_t;
  using internal::tuple_contains;
  using internal::tuple_contains_v;
  using internal::double_to_fraction;
  using internal::license;
}

#endif // BLACK_COMMON_H
