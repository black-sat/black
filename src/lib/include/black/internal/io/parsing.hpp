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

#ifndef BLACK_INTERNAL_IO_PARSING_HPP
#define BLACK_INTERNAL_IO_PARSING_HPP

#include <black/support>
#include <black/ast/core>

#include <cctype>
#include <cstdlib>
#include <iostream>

namespace black::io::parsing {

  template<typename ...Ts>
  struct success { 
    const char * current;
    std::tuple<Ts ...> values;

    success(const char * current, Ts ...values) 
      : current{current}, values{std::move(values)...} { }
    
    success(const char * current, std::tuple<Ts ...> values) 
      : current{current}, values{std::move(values)} { }

    bool operator==(success const&) const = default;
  };
  
  struct eof { 
    bool operator==(eof const&) const = default;
  };
  
  struct not_me { 
    bool operator==(not_me const&) const = default;
  };
  
  struct error {
    const char *current;
    std::string msg; // TODO: enrich errors

    bool operator==(error const&) const = default;
  };

  template<typename ...Ts>
  class result 
  {
  public:
    using result_t = std::variant<success<Ts...>, eof, not_me, error>;

    template<typename T>
      requires std::is_constructible_v<result_t, T>
    result(T&& v) : _data{std::forward<T>(v)} { }

    template<typename T>
    bool is() const {
      if constexpr(support::is_in_variant_v<T, result_t>)
        return std::holds_alternative<T>(_data);
      return false;
    }

    template<typename T>
    std::optional<T> get() const {
      if constexpr(support::is_in_variant_v<T, result_t>)
        if(is<T>())
          return std::get<T>(_data);
      return {};
    }

    template<
      typename F, 
      typename R = std::invoke_result_t<F, Ts...>,
      typename Result = 
        std::conditional_t<
          std::is_same_v<R, void>, result<>, result<R>
        >
    >
    Result map(F f) const {
      return support::match(_data)(
        [&](success<Ts...> s) -> Result {
          return std::apply([&](Ts ...vs) {
            if constexpr(std::is_same_v<R, void>) {
              f(std::move(vs)...);
              return success{s.current};
            } else {
              return success{s.current, f(std::move(vs)...)};
            }
          }, std::move(s.values));
        },
        [](auto r) -> Result { return r; }
      );
    }

    template<
      typename F, typename R = std::invoke_result_t<F, success<Ts...>, Ts...>
    >
    R then(F f) const {
      return support::match(_data)(
        [&](success<Ts ...> s) -> R {
          return std::apply([&](Ts ...vs) {
            return f(std::move(s), std::move(vs)...);
          }, std::move(s.values));
        },
        [](auto r) -> R { return r; }
      );
    }

  private:
    result_t _data;
  };

  template<typename ...Ts>
  result(success<Ts...>) -> result<Ts...>;

}

namespace black::support {

  template<typename ...Ts>
  struct match_cases<black::io::parsing::result<Ts...>> 
    : support::variant_alternatives<
        typename black::io::parsing::result<Ts...>::result_t
      > { };

  template<typename T, typename ...Ts>
    requires 
      support::is_in_variant_v<
        T, typename black::io::parsing::result<Ts...>::result_t
      >
  struct match_downcast<black::io::parsing::result<Ts...>, T> {
    static std::optional<T> downcast(black::io::parsing::result<Ts...> v) {
      return v.template get<T>();
    }
  };
}

namespace black::io::parsing {

  template<typename T>
  struct to_result { };

  template<typename ...Ts>
  struct to_result<std::tuple<Ts...>> : std::type_identity<result<Ts...>> { };

  template<typename ...Ts>
  using to_result_t = typename to_result<Ts...>::type;

  template<typename P>
  concept parser = requires (P p, const char * s) {
    typename P::parsed_types;
    { p.parse(s, s) } -> std::same_as<to_result_t<typename P::parsed_types>>;
  };

  struct string {

    using parsed_types = std::tuple<>;

    string(const char *str) : _str{str}, _len{std::strlen(str)} { }

    result<> parse(const char *begin, const char *end) {
      if(begin >= end)
        return eof{};

      if(size_t(end - begin) < _len)
        return not_me{};

      std::string_view view{begin, begin + _len};
      if(view == _str)
        return success{begin + _len};
      
      return not_me{};
    }
    
    const char *_str;
    size_t _len;
  };


  template<typename F>
  struct lexeme {

    using parsed_types = std::tuple<std::string_view>;

    lexeme(F f) : _pred{f} { }

    result<std::string_view> parse(const char *begin, const char *end) {
      if(begin >= end)
        return eof{};

      auto last = begin;
      while(last != end && _pred(*last))
        last++;
      
      if(last == begin)
        return not_me{};
      
      return success{last, std::string_view{begin, last}};
    }

    F _pred;
  };

  template<typename F>
  lexeme(F) -> lexeme<F>;

  template<
    typename First, typename Second, 
    typename FResult = typename First::parsed_types, 
    typename SResult = typename Second::parsed_types
  >
  struct seq;
  
  template<
    typename P1, typename P2,
    typename ...Types1, typename ...Types2
  >
  struct seq<P1, P2, std::tuple<Types1...>, std::tuple<Types2...>>
  { 
    using parsed_types = std::tuple<Types1..., Types2...>;
    
    seq(P1 first, P2 second) : _first{first}, _second{second} { }

    auto parse(const char *begin, const char *end) {
      return _first.parse(begin, end).then([&](auto s1, auto ...vs1) {
        return _second.parse(s1.current, end).then([&](auto s2, auto ...vs2) {
          return result{success{s2.current, vs1..., vs2...}};
        });
      });
    }

    P1 _first;
    P2 _second;
  };

  template<typename T1, typename T2>
  seq(T1, T2) -> seq<T1, T2>;  

  template<parser P1, parser P2>
  seq<P1, P2> operator+(P1 p1, P2 p2) {
    return seq(std::move(p1), std::move(p2));
  }

  template<typename P1, typename P2>
    requires std::same_as<typename P1::parsed_types, typename P2::parsed_types>
  struct either {
    using parsed_types = typename P1::parsed_types;
    
    either(P1 first, P2 second) : _first{first}, _second{second} { }

    auto parse(const char *begin, const char *end) {
      return support::match(_first.parse(begin, end))(
        []<typename ...Ts>(success<Ts ...> s) { return result{std::move(s)}; },
        [&](not_me) { return _second.parse(begin, end); },
        [](auto r) { return r; }
      );
    }

    P1 _first;
    P2 _second;
  };

  template<typename T1, typename T2>
  either(T1, T2) -> either<T1, T2>;  

  template<parser P1, parser P2>
  either<P1, P2> operator|(P1 p1, P2 p2) {
    return either(std::move(p1), std::move(p2));
  }

  template<typename P, typename F, typename = typename P::parsed_types>
  struct act;

  template<typename P, typename F, typename ...PTypes>
  struct act<P, F, std::tuple<PTypes...>>
  {
    using parsed_types = std::tuple<std::invoke_result_t<F, PTypes...>>;
    
    act(P parser, F func) : _parser{parser}, _func{func} { }

    auto parse(const char *begin, const char *end) {
      return _parser.parse(begin, end).map(_func);
    }

    P _parser;
    F _func;
  };

  template<typename T, typename F>
  act(T, F) -> act<T, F>;

}



#endif // BLACK_INTERNAL_IO_PARSING_HPP
