//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_INTERNAL_PARSING_RESULT_HPP
#define BLACK_INTERNAL_PARSING_RESULT_HPP

#include <black/support>
#include <black/ast/core>

#include <cctype>
#include <cstdlib>
#include <iostream>

namespace black::parsing {

  template<std::forward_iterator It, typename ...Ts>
  class result;

  template<std::forward_iterator It, typename ...Ts>
  struct success { 
    It current;
    std::tuple<Ts ...> values;

    explicit success(It current, Ts ...values) 
      : current{current}, values{std::move(values)...} { }
    
    success(It current, std::tuple<Ts ...> values) 
      : current{current}, values{std::move(values)} { }
    
    success(success const&) = default;
    success(success &&) = default;

    success &operator=(success const&) = default;
    success &operator=(success &&) = default;

    bool operator==(success const& other) const = default;
  };

  template<size_t I, typename It, typename ...Ts>
    requires (I < sizeof...(Ts) + 1)
  auto const& get(success<It, Ts...> const& s) {
    if constexpr(I == 0)
      return s.current;
    else
      return std::get<I - 1>(s.values);    
  }
  
  template<size_t I, typename It, typename ...Ts>
    requires (I < sizeof...(Ts) + 1)
  auto && get(success<It, Ts...> && s) {
    if constexpr(I == 0)
      return std::move(s.current);
    else
      return std::move(std::get<I - 1>(s.values));
  }

  template<std::forward_iterator It>
  struct failure {
    It current;

    explicit failure(It current) : current{current} { }

    failure(failure const&) = default;
    failure(failure &&) = default;

    failure &operator=(failure const&) = default;
    failure &operator=(failure &&) = default;

    bool operator==(failure const& other) const = default;

  };

  template<size_t I, typename It>
    requires (I == 0)
  auto get(failure<It> f) {
    return f.current;
  }

}

template<typename It, typename ...Ts>
struct std::tuple_size<black::parsing::success<It, Ts...>>
  : std::integral_constant<size_t, sizeof...(Ts) + 1> { };

template<typename It, typename ...Ts>
struct std::tuple_element<0, black::parsing::success<It, Ts...>> 
  : std::type_identity<It> { };

template<size_t I, typename It, typename ...Ts>
struct std::tuple_element<I, black::parsing::success<It, Ts...>> 
  : std::tuple_element<I - 1, std::tuple<Ts...>> { };

template<typename It>
struct std::tuple_size<black::parsing::failure<It>>
  : std::integral_constant<size_t, 1> { };

template<typename It>
struct std::tuple_element<0, black::parsing::failure<It>> 
  : std::type_identity<It> { };

namespace black::support {

  template<typename It, typename ...Ts>
  struct common_result<
    black::parsing::success<It, Ts...>, 
    black::parsing::failure<It>
  > : std::type_identity<black::parsing::result<It, Ts...>> { };
  
  template<typename It, typename ...Ts>
  struct common_result<
    black::parsing::failure<It>,
    black::parsing::success<It, Ts...>
  > : std::type_identity<black::parsing::result<It, Ts...>> { };

}


namespace black::parsing {

  template<std::forward_iterator It, typename ...Ts>
  class result 
  {
  public:
    using success_t = parsing::success<It, Ts...>;
    using failure_t = parsing::failure<It>;

    result(success_t s) : _data{s} { }
    result(failure_t f) : _data{f} { }

    result(result const&) = default;
    result(result &&) = default;
    
    result &operator=(result const&) = default;
    result &operator=(result &&) = default;

    std::optional<success_t> success() const {
      if(std::holds_alternative<success_t>(_data))
        return std::get<success_t>(_data);
      return {};
    }
    
    std::optional<failure_t> failure() const {
      if(std::holds_alternative<failure_t>(_data))
        return std::get<failure_t>(_data);
      return {};
    }

    template<typename F>
    auto map(F func) const {
      return support::match(_data)(
        [&](success_t, auto it, Ts ...vs) {
          if constexpr(std::same_as<std::invoke_result_t<F, Ts...>, void>) {
            func(std::move(vs)...);
            return parsing::success{it};
          } else {
            return parsing::success{it, func(std::move(vs)...)};
          }
        },
        [](failure_t f) { return f; }
      );
    }

    template<typename F>
    auto then(F func) const {
      return support::match(_data)(
        [&](success_t, It it, Ts ...vs) {
          return func(std::move(it), std::move(vs)...);
        },
        [](failure_t f) { return f; }
      );
    }

    explicit operator bool() const {
      return std::holds_alternative<success_t>(_data);
    }

  private:
    std::variant<success_t, failure_t> _data;
  };

}

namespace black::support {

  template<typename It, typename ...Ts>
  struct match_cases<parsing::result<It, Ts...>> 
    : std::type_identity<
        std::tuple<parsing::success<It, Ts...>, parsing::failure<It>>
      > { };
  
  template<typename It, typename ...Ts>
  struct 
  match_downcast<parsing::result<It, Ts...>, parsing::success<It, Ts...>> {
    static auto downcast(parsing::result<It, Ts...> r) {
      return r.success();
    }
  };
  
  template<typename It, typename ...Ts>
  struct 
  match_downcast<parsing::result<It, Ts...>, parsing::failure<It>> {
    static auto downcast(parsing::result<It, Ts...> r) {
      return r.failure();
    }
  };

}


#endif // BLACK_INTERNAL_PARSING_RESULT_HPP
