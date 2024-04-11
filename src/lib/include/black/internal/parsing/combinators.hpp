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

namespace black::parsing {

  using support::match;

  //!
  //! Traits to get info about parsers
  //!

  template<typename P, typename It, typename End>
  struct parser_result { };

  template<typename P, typename It, typename End>
    requires (
      std::forward_iterator<It> && std::sentinel_for<End, It> &&
      std::invocable<P, It, End>
    )
  struct parser_result<P, It, End> : std::invoke_result<P, It, End> { };

  template<typename P, typename It, typename End>
  using parser_result_t = typename parser_result<P, It, End>::type;

  template<typename R>
  struct parser_result_output { };

  template<typename It, typename ...Ts>
  struct parser_result_output<result<It, Ts...>>
    : std::type_identity<std::tuple<Ts...>> { };

  template<typename R>
  using parser_result_output_t = typename parser_result_output<R>::type;

  template<typename P, typename It, typename End>
  struct parser_output 
    : parser_result_output<parser_result_t<P, It, End>> { };

  template<typename P, typename It, typename End>
  using parser_output_t = typename parser_output<P, It, End>::type;

  template<typename It, typename T>
  struct result_from { };

  template<std::forward_iterator It, typename ...Ts>
  struct result_from<It, std::tuple<Ts...>> 
    : std::type_identity<result<It, Ts...>> { };

  template<typename It, typename T>
  using result_from_t = typename result_from<It, T>::type;

  //!
  //! Matches a single element with no output value.
  //!
  template<typename T>
  inline auto symbol(T v) {
    return [=]<typename It>(It begin, auto end) -> result<It>
    {
        if(begin != end && *begin == v)
          return success{begin + 1};

        return failure{begin};
      };
  }

  //!
  //! Matches a sequence of one or more elements satisfying `pred`.
  //! The output value is the matched subrange.
  //!
  template<typename F>
  inline auto pattern(F pred) {
    return [=]<typename It>(It begin, auto end) 
      -> result<It, std::ranges::subrange<It, It>>
    {
      auto it = begin;
      while(it != end && pred(*it))
        it++;
      
      if(it == begin)
        return failure{it};

      return success{it, std::ranges::subrange(begin, it)};
    };
  }

  //!
  //! Matches a sequence of parsers and fails if any of the sequence fails/
  //! The output value is the tuple of output values of each parser of the 
  //! sequence.
  //!
  inline auto seq(auto p) { return p; }

  inline auto seq(auto p1, auto p2) {
    return [=](auto begin, auto end) {
      return p1(begin, end).then([&](auto it1, auto ...vs1) {
        return p2(it1, end).then([&](auto it2, auto ...vs2) {
          return success{it2, std::move(vs1)..., std::move(vs2)...};
        });
      });
    };
  }
  
  inline auto seq(auto p1, auto p2, auto ...ps) {
    return seq(std::move(p1), seq(std::move(p2), std::move(ps)...));
  }

  //!
  //! Matches the first parser and, only if the first fails without consuming
  //! any input, matches the second.
  //!
  inline auto either(auto p1, auto p2) {
    return [=]<typename It>(It begin, auto end) -> result<It> {
      auto r1 = p1(begin, end);
      if(auto s = r1.success(); s)
        return result{*s};
      
      auto it = r1.failure()->current;
      if(it != begin)
        return *r1.failure();
      
      return p2(it, end);
    };
  }

  //!
  //! Matches the given parser and, if it fails, does not consume input.
  //!
  inline auto trying(auto p) {
    return [=](auto begin, auto end) {
      return match(p(begin, end))(
        [=]<typename It>(failure<It>) {
          return failure<It>{begin};
        },
        [](auto s) { return s; }
      );
    };
  }

  //!
  //! Matches exactly the range given as input. Consumes the input until the end
  //! of the matched string or the first different character. There is no
  //! output.
  //!
  inline auto exactly(std::ranges::range auto r) {    
    return [p = std::move(r)]<typename It>(It b, auto end) -> result<It> {
      auto it = b;
      auto it2 = std::ranges::begin(p);

      while(it != end && it2 != std::ranges::end(p) && *it == *it2) {
        it++;
        it2++;
      }

      if(it2 == std::ranges::end(p))
        return success{it};
      
      return failure{it};
    };
  }

  inline auto exactly(std::string_view str) {
    return exactly(std::ranges::subrange(begin(str), end(str)));
  }

  //!
  //! Transform the output value of a matched parser, fails if the parser fails.
  //!
  inline auto apply(auto p, auto f) {
    return [=](auto begin, auto end) -> decltype(p(begin, end).map(f)) {
      return p(begin, end).map(f);
    };
  }

  //!
  //! Matches the given parser and, if successful, filters it with the predicate
  //! and fails if it returns false.
  //!
  inline auto filter(auto p, auto f) {
    return [=](auto begin, auto end) {
      return p(begin, end).then(
        [&]<typename It, typename ...Ts>(It it, Ts ...vs) -> result<It, Ts...> {
          if(f(vs...))
            return success{it, std::move(vs)...};
          return failure{it};
        }
      );
    };
  }

  //! 
  //! Utility for many() below.
  //! 
  template<typename T, template<typename ...> class F>
  struct tuple_map { };

  template<typename ...Ts, template<typename ...> class F>
  struct tuple_map<std::tuple<Ts...>, F> 
    : std::type_identity<std::tuple<F<Ts>...>> { };

  template<typename T, template<typename ...> class F>
  using tuple_map_t = typename tuple_map<T, F>::type;
  

  //!
  //! Matches zero or more matches of the given parser.
  //! Outputs a tuple of vectors of the matches' outputs.
  //!
  template<typename P>
  auto many(P p) {
    return [=]<
        typename It, typename End,
        typename R = tuple_map_t<parser_output_t<P, It, End>, std::vector>
      >(It begin, End end) -> result_from_t<It, R> 
    {
      R res;

      auto it = begin;
      auto parsed = p(it, end);
      while(parsed) {
        std::apply([&]<typename ...Ts>(Ts ...values) {
          [&]<size_t ...Idx>(std::index_sequence<Idx...>) {
            (std::get<Idx>(res).push_back(values), ...);
          }(std::index_sequence_for<Ts...>{});
        }, std::move(parsed.success()->values));

        it = parsed.success()->current;
        begin = it;
        parsed = p(it, end);
      }

      if(it == begin)
        return success{it, res};

      return failure{it};
    };
  }

  //!
  //! Utility for optional() below
  //!
  template<typename T, typename Tu>
  struct tuple_cons { };

  template<typename T, typename ...Ts>
  struct tuple_cons<T, std::tuple<Ts...>> 
    : std::type_identity<std::tuple<T, Ts...>> { };

  template<typename T, typename Tu>
  using tuple_cons_t = typename tuple_cons<T, Tu>::type;

  
  template<typename P, typename It, typename End>
  using optional_output_t = 
    tuple_cons_t<
      bool, tuple_map_t<parser_output_t<P, It, End>, std::optional>
    >;

  //!
  //! Matches a parser optionally. Fails only if the parser fails after
  //! consuming input. Outputs the parsers' output with the addition of a
  //! boolean flag.
  //!
  template<typename P>
  inline auto optional(P p) {
    return [=]<
        typename It, typename End, typename R = optional_output_t<P, It, End>  
      >(It begin, End end) -> result_from_t<It, R>
    {
      return match(p(begin, end))(
        []<typename ...Ts>(success<It, Ts...>, It it, Ts ...vs) {
          return success(it, true, std::optional<Ts>{vs}...);
        },
        [&](failure<It> f, It it) -> result_from_t<It, R> {
          if(it != begin)
            return f;
          return success{it, R{}};
        }
      );
    };
  }

  //!
  //! Matches a parser and throws away the output value.
  //!
  inline auto ignore(auto p) {
    return apply(std::move(p), [](auto...){ });
  }

  //!
  //! Matches one or more matches of the given parser separated by the given
  //! separator parser. The output is a vector of all the matches.
  //!
  inline auto sep_by(auto parser, auto sep) {
    return apply(
      seq(parser, many(seq(sep, parser))),
      [](auto v, auto vs) {
          vs.insert(begin(vs), v);
          return vs;
      }
    );
  }

  //
  // Combinators specific to character iterators
  //

  //!
  //! Same as `pattern()` but works on contiguous ranges of chars and the output
  //! value is a `std::string_view`.
  //!
  inline auto string(auto f) {
    return apply(pattern(f), [](std::ranges::contiguous_range auto rng) {
      return std::string_view{
        std::to_address(std::ranges::begin(rng)), 
        std::to_address(std::ranges::end(rng))
      };
    });
  }

  //!
  //! Matches and ignores any amount of spaces, if any.
  //!
  inline auto spaces() {
    return ignore(optional(pattern(&isspace)));
  }

  //!
  //! Matches the given parser ignoring any subsequent space, if any.
  //!
  inline auto token(auto p) {
    return seq(p, spaces());
  }

}

#endif // BLACK_INTERNAL_PARSING_COMBINATORS_HPP
