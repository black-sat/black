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
#include <memory>
#include <string>
#include <optional>
#include <typeinfo>
#include <limits>
#include <cmath>

#ifdef _MSC_VER
  #define BLACK_EXPORT __declspec(dllexport)
#else
  #define BLACK_EXPORT
#endif

#if defined(_MSC_VER)
  #include <BaseTsd.h>
  using ssize_t = SSIZE_T;
#endif

// for abi::__cxa_demangle to demangle names coming from type_info::name()
#if __has_include(<cxxabi.h>)
  #include <cxxabi.h>
#endif

namespace black::support::internal 
{
  //
  // Wrapper over `abi::__cxa_demangle()` used later in `bad_pattern`.
  // If <cxxabi.h> is not there (any compiler other than GCC and clang),
  // we just return the string as-is.
  //
  inline std::string demangle(const char *name) {
    #if __has_include(<cxxabi.h>)
      int status = 0;
      
      auto result = std::unique_ptr<char, decltype(&free)>(
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        &free
      );
      
      return result.get();
    #else
      return name;
    #endif
  }

  template<typename T>
  std::string type_name() {
    return demangle(typeid(T).name());
  }

  inline std::string to_camel(std::string_view name) {
    std::string str;

    bool capital = true;
    for(char c : name) {
      if(c != '_')
        str += capital ? (char)toupper(c) : c;
      capital = (c == '_');
    }
    
    return str;
  }

  //
  // Utilities for variants
  //

  template<typename T>
  struct variant_alternatives { };

  template<typename ...Ts>
  struct variant_alternatives<std::variant<Ts...>> 
    : std::type_identity<std::tuple<Ts...>> { };

  template<typename T>
  using variant_alternatives_t = typename variant_alternatives<T>::type;
  
  template<typename T, typename V>
  struct is_in_variant : std::false_type { };

  template<typename T, typename ...Ts>
  struct is_in_variant<T, std::variant<Ts...>>
    : std::bool_constant<(std::same_as<T, Ts> || ...)> { };

  template<typename T, typename V>
  inline constexpr bool is_in_variant_v = is_in_variant<T, V>::value;


  inline constexpr std::string_view license =
R"(
BLACK - Bounded Lᴛʟ sAtisfiability ChecKer

(C) 2019-2024 Nicola Gigante
    2019-2024 Luca Geatti
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
  using internal::type_name;
  using internal::demangle;
  using internal::to_camel;
  using internal::variant_alternatives;
  using internal::variant_alternatives_t;
  using internal::is_in_variant;
  using internal::is_in_variant_v;
  using internal::to_camel;
  using internal::license;
}

#endif // BLACK_COMMON_H
