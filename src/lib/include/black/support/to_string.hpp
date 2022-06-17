//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_TO_STRING_HPP
#define BLACK_TO_STRING_HPP

#include <type_traits>
#include <string>
#include <string_view>
#include <tuple>

#include <black/support/meta.hpp>

namespace black::internal {

  using std::to_string;

  template<typename T>
  concept stringable = requires(T t) {
    { to_string(t) } -> std::convertible_to<std::string>;
  };

  template<typename T>
    requires (std::integral<T> || std::floating_point<T>)
  inline std::string to_string(T v) {
    return std::to_string(v);
  }

  inline std::string to_string(std::string const&s) {
    return s;
  }

  inline std::string to_string(std::string_view const&sv) {
    return std::string{sv};
  }

  template<stringable T, stringable U>
  std::string to_string(std::pair<T, U> const&p) {
    return to_string(p.first) + ", " + to_string(p.second);
  }

  inline std::string to_string(std::tuple<> const&) {
    return "";
  }

  template<stringable T>
  std::string to_string(std::tuple<T> const& t) {
    return to_string(std::get<0>(t));
  }

  template<stringable T, typename ...Args>
  std::string to_string(std::tuple<T, Args...> const & t) {
    return std::apply([](auto v, auto ...vs) {
      return v + ((", " + to_string(vs)) + ...);
    }, t);
  }
}

namespace black {
  using internal::to_string;
}

#endif
