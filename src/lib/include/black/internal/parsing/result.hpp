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

  template<typename Out>
  class result;

  template<typename Out>
  struct success { 
    const char *current;
    Out output;

    success(const char *current, Out out) 
      : current{current}, output{std::move(out)} { }
    
    success(const char *current) : current{current} { }

    success(success const&) = default;
    success(success &&) = default;
    
    success &operator=(success const&) = default;
    success &operator=(success &&) = default;

    bool operator==(success const& other) const = default;
  };

  template<typename Out>
  success(const char *, Out) -> success<Out>;
  
  success(const char *) -> success<std::monostate>;

  struct failure {
    const char *current;

    bool operator==(failure const& other) const = default;
  };

  template<size_t I, typename Out>
  auto & get(success<Out> & s) {
    if constexpr(I == 0)
      return s.current;
    else
      return s.output;
  }

  template<size_t I, typename Out>
  auto const& get(success<Out> const& s) {
    if constexpr(I == 0)
      return s.current;
    else
      return s.output;
  }

  
  template<size_t I>
    requires (I == 0)
  auto get(failure f) {
    return f.current;
  }

}

template<typename Out>
struct std::tuple_size<black::parsing::success<Out>>
  : std::integral_constant<size_t, 2> { };

template<typename Out>
struct std::tuple_element<0, black::parsing::success<Out>> 
  : std::type_identity<const char *> { };

template<typename Out>
struct std::tuple_element<1, black::parsing::success<Out>> 
  : std::type_identity<Out> { };

template<>
struct std::tuple_size<black::parsing::failure>
  : std::integral_constant<size_t, 1> { };

template<>
struct std::tuple_element<0, black::parsing::failure> 
  : std::type_identity<const char *> { };

namespace black::support {

  template<typename Out>
  struct common_result<
    black::parsing::success<Out>, 
    black::parsing::failure
  > : std::type_identity<black::parsing::result<Out>> { };
  
  template<typename Out>
  struct common_result<
    black::parsing::failure,
    black::parsing::success<Out>
  > : std::type_identity<black::parsing::result<Out>> { };

}

namespace black::parsing {

  template<typename Out>
  class result 
  {
  public:
    using output_t = Out;

    result(parsing::success<Out> s) : _data{s} { }
    result(parsing::failure f) : _data{f} { }

    result(result const&) = default;
    result(result &&) = default;
    
    result &operator=(result const&) = default;
    result &operator=(result &&) = default;

    std::optional<parsing::success<Out>> success() const {
      if(std::holds_alternative<parsing::success<Out>>(_data))
        return std::get<parsing::success<Out>>(_data);
      return {};
    }
    
    std::optional<parsing::failure> failure() const {
      if(std::holds_alternative<parsing::failure>(_data))
        return std::get<parsing::failure>(_data);
      return {};
    }

    template<typename F>
    auto map(F func) const {
      return support::match(_data)(
        [&](parsing::success<Out>, const char *p, Out out) {
          return parsing::success{p, func(std::move(out))};
        },
        [](parsing::failure f) { return f; }
      );
    }

    template<typename F>
    auto then(F func) const {
      return support::match(_data)(
        [&](parsing::success<Out>, const char *p, Out out) {
          return func(p, std::move(out));
        },
        [](parsing::failure f) { return f; }
      );
    }

    template<typename F>
    auto or_else(F func) const {
      return support::match(_data)(
        [&](parsing::failure, const char *p) {
          return func(p);
        },
        [](auto s) { return std::move(s); }
      );
    }

    explicit operator bool() const {
      return std::holds_alternative<parsing::success<Out>>(_data);
    }

  private:
    std::variant<parsing::success<Out>, parsing::failure> _data;
  };

}

namespace black::support {
  template<typename Out>
  struct match_cases<parsing::result<Out>> 
    : std::type_identity<
        std::tuple<parsing::success<Out>, parsing::failure>
      > { };
  
  template<typename Out>
  struct 
  match_downcast<parsing::result<Out>, parsing::success<Out>> {
    static auto downcast(parsing::result<Out> r) {
      return r.success();
    }
  };
  
  template<typename Out>
  struct 
  match_downcast<parsing::result<Out>, parsing::failure> {
    static auto downcast(parsing::result<Out> r) {
      return r.failure();
    }
  };
}


#endif // BLACK_INTERNAL_PARSING_RESULT_HPP
