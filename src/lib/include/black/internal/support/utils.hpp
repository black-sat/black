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
  template<typename T>
  std::string type_name() {
    #if __has_include(<cxxabi.h>)
      int status = 0;
      
      auto result = std::unique_ptr<char, decltype(&free)>(
        abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
        &free
      );
      
      return result.get();
    #else
      return typeid(T).name();
    #endif
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
  using internal::type_name;
  using internal::double_to_fraction;
  using internal::license;
}

#endif // BLACK_COMMON_H