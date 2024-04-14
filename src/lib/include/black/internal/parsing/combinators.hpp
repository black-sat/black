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

#ifndef BLACK_INTERNAL_PARSING_COMBINATORS_HPP
#define BLACK_INTERNAL_PARSING_COMBINATORS_HPP

#include <cctype>
#include <ranges>

namespace black::parsing {

  using support::match;
  using support::matching;

  //! 
  //! Traits and concepts
  //! 
  template<typename P>
  struct parser_output { };

  template<typename R>
  struct parser_output_aux { };

  template<typename Out>
  struct parser_output_aux<result<Out>> 
    : std::type_identity<Out> { };

  template<typename P>
    requires std::invocable<P, const char *, const char *>
  struct parser_output<P> 
    : parser_output_aux<std::invoke_result_t<P, const char *, const char *>>
  { };
  
  template<typename P>
    requires std::invocable<P, P, const char *, const char *>
  struct parser_output<P> 
    : parser_output_aux<std::invoke_result_t<P, P, const char *, const char *>>
  { };

  template<typename P>
  using parser_output_t = typename parser_output<P>::type;

  template<typename P>
  concept parser = requires { 
    typename parser_output<P>::type;
  };

  template<typename F, typename P>
  struct is_applicable_on_parser : std::false_type { };

  template<typename F, typename P>
    requires parser<P>
  struct is_applicable_on_parser<F, P> :
    std::is_invocable<F, parser_output_t<P>> { };

  template<typename F, typename P>
  inline constexpr bool is_applicable_on_parser_v = 
    is_applicable_on_parser<F, P>::value;

  template<typename F, typename P>
  concept applicable_on_parser = is_applicable_on_parser_v<F, P>;

  // template<typename R, template<typename ...> class T>
  // struct wrap_results_with { };
  
  // template<typename ...Outs, template<typename ...> class T>
  // struct wrap_results_with<result<Outs...>, T>
  //   : std::type_identity<result<T<Outs>...>> { };

  // template<typename R, template<typename ...> class T>
  // using wrap_results_with_t = typename wrap_results_with<R, T>::type;

  inline constexpr auto concat(auto v1, auto v2) {
    return std::tuple{std::move(v1), std::move(v2)};
  }

  inline constexpr auto concat(auto v1, std::monostate) {
    return v1;
  }
  
  inline constexpr auto concat(std::monostate, auto v2) {
    return v2;
  }
  
  inline constexpr auto concat(std::monostate, std::monostate) {
    return std::monostate{};
  }

  template<typename ...Ts>
  inline constexpr auto concat(auto v1, std::tuple<Ts...> t) {
    return std::apply([=](Ts ...vs) {
      return std::tuple{v1, vs...};
    }, std::move(t));
  }

  template<typename ...Ts>
  inline constexpr auto concat(std::tuple<Ts...> t, auto v2) {
    return std::apply([=](Ts ...vs) {
      return std::tuple{vs..., v2};
    }, std::move(t));
  }

  template<typename ...Ts1, typename ...Ts2>
  inline constexpr auto concat(std::tuple<Ts1...> t1, std::tuple<Ts2...> t2) {
    return std::tuple_cat(std::move(t1), std::move(t2));
  }

  inline constexpr auto clean(auto v) {
    return v;
  }

  template<typename T>
  inline constexpr auto clean(std::tuple<T> t) {
    return clean(std::move(std::get<0>(t)));
  }

  template<typename T>
  inline constexpr auto clean(std::variant<T> v) {
    return clean(std::move(std::get<T>(v)));
  }

  template<typename T1, typename T2>
  struct choice : std::type_identity<std::variant<T1, T2>> { };
  
  template<typename T1, typename T2>
  using choice_t = typename choice<T1, T2>::type;
  
  template<typename T>
  struct choice<T, T> : std::type_identity<T> { };

  template<typename T1, typename ...Ts2>
  struct choice<T1, std::variant<Ts2...>>
    : std::conditional<
        (std::is_same_v<T1, Ts2> || ...), 
        std::variant<Ts2...>,
        std::variant<T1, Ts2...>
      > { };

  template<typename ...Ts1, typename T2>
  struct choice<std::variant<Ts1...>, T2>
    : std::conditional<
        (std::is_same_v<Ts1, T2> || ...),
        std::variant<Ts1...>,
        std::variant<Ts1..., T2>
      > { };

  template<typename ...Ts1, typename T2>
  struct choice<std::variant<Ts1...>, std::variant<T2>>
    : choice<std::variant<Ts1...>, T2> { };

  template<typename ...Ts1, typename T2, typename ...Ts2>
  struct choice<std::variant<Ts1...>, std::variant<T2, Ts2...>> 
    : choice<choice_t<std::variant<Ts1...>, T2>, std::variant<Ts2...>> { };

  template<typename T>
  auto upcast(auto v) {
    return T{std::move(v)};
  }

  template<typename T, typename ...Ts>
  auto upcast(std::variant<Ts...> v) {
    return match(v)(
      [](auto vv) {
        return T{vv};
      }
    );
  }

  //!
  //! Wrapper type for parsers.
  //!
  template<typename P>
  class wrap
  {
  public:
    constexpr wrap(P p) : _parser{std::move(p)} { }

    template<std::contiguous_iterator It, std::sentinel_for<It> End>
      requires std::same_as<std::remove_const_t<std::iter_value_t<It>>, char>
    auto operator()(It begin, End end) const {
      return _parser(std::to_address(begin), std::to_address(end));
    }
    
    template<std::ranges::contiguous_range Rng>
      requires 
        std::same_as<std::remove_const_t<std::ranges::range_value_t<Rng>>, char>
    auto operator()(Rng const&rng) const {
      return (*this)(std::ranges::cbegin(rng), std::ranges::cend(rng));
    }

  private:
    P _parser;
  };

  //!
  //! Always succeeds.
  //!
  inline constexpr auto succeed() {
    return wrap{
      [=](const char *begin, const char *) -> result<std::monostate> {
        return success{begin, std::monostate{}};
      }
    };
  }
  
  
  //!
  //! Always fails.
  //!
  inline constexpr auto fail() {
    return wrap{
      [=](const char *begin, const char *) -> result<std::monostate> {
        return failure{begin};
      }
    };
  }

  //!
  //! Matches a sequence of parsers failing if any one fails.
  //! Output: all the outputs of the sequence elements.
  //!
  inline constexpr auto seq(parser auto p1, parser auto p2) {
    return wrap{
      [=](const char *begin, const char *end) {
        return p1(begin, end).then([&](const char *it1, auto out1) {
          return p2(it1, end).then([&](const char *it2, auto out2) {
            return 
              success{it2, clean(concat(std::move(out1), std::move(out2)))};
          });
        });
      }
    };
  }

  template<typename ...Ps>
    requires (sizeof...(Ps) >= 2)
  inline constexpr auto seq(auto p1, Ps ...ps) {
    return seq(p1, seq(ps...));
  }

  //!
  //! Matches the parsers in the sequence in order, trying the next if one fails
  //! without consuming inputs. It fails if one fails consuming input or if no
  //! one succeeds.
  //! Output: the output of the matching parser.
  //!
  template<
    parser P1, parser P2,
    typename Out1 = parser_output_t<P1>,
    typename Out2 = parser_output_t<P2>,
    typename R = choice_t<Out1, Out2>
  >
  inline constexpr auto either(P1 p1, P2 p2) {
    return wrap{
      [=](const char *begin, const char *end) -> result<R> {
        return match(p1(begin, end))(
          [](success<Out1>, const char *it, Out1 out) -> result<R> {
            return success{it, upcast<R>(std::move(out))};
          },
          [&](failure, const char *it1) -> result<R> {
            if(it1 != begin)
              return failure{it1};
            return match(p2(begin, end))(
              [](success<Out2>, const char *it2, Out2 out) -> result<R> {
                return success{it2, upcast<R>(std::move(out))};
              },
              [](failure f) -> result<R> {
                return f;
              }
            );
          }
        );
      }
    };
  }

  template<typename ...Ps>
    requires (sizeof...(Ps) >= 2)
  inline constexpr auto either(parser auto p1, parser auto ...ps) {
    return either(p1, either(ps...));
  }

  //!
  //! Matches the given parser transforming the outputs by the given function
  //!
  template<parser P, applicable_on_parser<P> F>
  inline constexpr auto apply(P p, F func) {
    return wrap{
      [=](const char *begin, const char *end) {
        return p(begin, end).map(func);
      }
    };
  }


  //! 
  //! Matches the given parser but accepts if it fails without consuming input.
  //! Output: an std::optional of a tuple containing the results.
  //!
  template<parser P>
  inline constexpr auto optional(P p) {
    return apply(
      either(p, succeed()),
      matching(
        [](std::monostate) { return std::nullopt; },
        [](auto v) { return std::optional{v}; }
      )
    );
  }

  // Waiting for 'deducing this' in clang 18
  template<typename P, typename Out>
  struct recursive_t {
    P _p;

    recursive_t(P p) : _p{std::move(p)} { }

    result<Out> operator()(const char *begin, const char *end) const {
      return _p(*this)(begin, end);
    }
  };

  //!
  //! Creates a recursive parser. See `many()` below as an example.
  //!
  template<typename Out, typename F>
  inline constexpr auto recursive(F f) {
    return wrap{ recursive_t<F, Out>{f} };
  }

  //!
  //! Matches one or more occurrences of the given parser. It fails if one fails
  //! after consuming input.
  //!
  template<typename P>
  inline constexpr auto many(P p) {
    using Out = std::deque<parser_output_t<P>>;

    return recursive<Out>([=](auto self) { 
      return apply(
        optional(seq(p, optional(self))),
        [](auto v) {
          if(!v)
            return Out{};
          
          return std::apply([](auto vv, auto opt){
            if(!opt)
              return std::deque{vv};
            opt->push_front(vv);
            return std::move(*opt);
          }, std::move(*v));
        }
      );
    });
  }

  //!
  //! Matches one or more occurrences of the given parser separated by the given
  //! separator parser.
  //!
  inline constexpr auto sep_by(parser auto p, parser auto sep) {
    return apply(
      seq(p, many(seq(sep, p))),
      [](auto t) {
        return std::apply([](auto head, auto tail) {
          tail.push_front(head);
          return tail;
        }, std::move(t));
      }
    );
  }

}

#endif // BLACK_INTERNAL_PARSING_COMBINATORS_HPP
