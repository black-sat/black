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

namespace black_internal {

  namespace to_string_details {
    using std::to_string;
    std::string to_string(std::string const&s);
    std::string to_string(std::string_view const&sv);

    template<typename T, typename U>
    std::string to_string(std::pair<T, U> const&p);

    std::string to_string(std::tuple<> const&);

    template<typename T>
    std::string to_string(std::tuple<T> const& t);

    template<typename T, typename ...Args>
    std::string to_string(std::tuple<T, Args...> const & t);

    struct to_string_niebloid {
      constexpr to_string_niebloid() = default;

      template<typename T>
      auto operator()(T&& v) const -> decltype(to_string(std::forward<T>(v))) {
        //using std::to_string;
        return to_string(std::forward<T>(v));
      }
    };

  }

  static constexpr to_string_details::to_string_niebloid to_string;

  template<typename T>
  concept stringable = requires(T t) {
    { to_string(t) } -> std::convertible_to<std::string>;
  };


  namespace to_string_details {

    inline std::string to_string(std::string const&s) {
      return s;
    }

    inline std::string to_string(std::string_view const&sv) {
      return std::string{sv};
    }

    template<typename T, typename U>
    std::string to_string(std::pair<T, U> const&p) {
      return black_internal::to_string(p.first) + ", " + 
             black_internal::to_string(p.second);
    }

    inline std::string to_string(std::tuple<> const&) {
      return "";
    }

    template<typename T>
    std::string to_string(std::tuple<T> const& t) {
      return black_internal::to_string(std::get<0>(t));
    }

    template<typename T, typename ...Args>
    std::string to_string(std::tuple<T, Args...> const & t) {
      return std::apply([](auto v, auto ...vs) {
        return 
          black_internal::to_string(v) + 
            ((", " + black_internal::to_string(vs)) + ...);
      }, t);
    }
  }

}

namespace black {
  using black_internal::to_string;
}

#endif
