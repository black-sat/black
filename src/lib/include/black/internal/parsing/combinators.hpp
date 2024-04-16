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

#include <expected>
#include <ranges>
#include <string>
#include <iostream>

namespace black::parsing {

  // should be in `std::ranges`
  template<typename Rng>
  using const_iterator_t = decltype(std::ranges::cbegin(std::declval<Rng&>()));
  
  template<typename Rng>
  using const_sentinel_t = decltype(std::ranges::cend(std::declval<Rng&>()));

  struct log {
    std::vector<std::string> errors; // TODO: to expand on this
  };

  template<typename T>
  struct success {
    T value;
    struct log log;
  };

  struct failure {
    enum { empty, error } kind;
    struct log log;
  };

  template<
    std::contiguous_iterator It, std::sentinel_for<It> End, typename State
  >
  struct state_t {
    It begin;
    End end;
    State state;
    struct log log;

    void append(struct log l2) {
      log.errors.insert(log.errors.end(), l2.errors.begin(), l2.errors.end());
    }
  };

  template<typename T>
  using result = std::expected<success<T>, failure>;
  
  //!
  //! Generic wrapper for parsers proving their user interface.
  //!
  template<typename In, typename State, typename Out, typename P>
  struct parser_t 
  {
    using input_type = In;
    using state_type = State;
    using output_type = Out;

    constexpr parser_t(P p) : run{std::move(p)} { }

    template<std::convertible_to<P> R>
    parser_t(parser_t<In, State, Out, R> r) : run{std::move(r.run)} { }

    template<std::contiguous_iterator It, std::sentinel_for<It> End>
    constexpr result<Out> operator()(It begin, End end, State init = {}) const 
    {
      using StateT = state_t<It, End, State>;

      return run(
        StateT{begin, end, std::move(init), log{}}, 
        [](Out in, StateT state) -> result<Out> { 
          return success{in, std::move(state.log)};            // success
        },
        [](log l) -> result<Out> { 
          return std::unexpected(failure{failure::empty, l});  // empty failure
        }, 
        [](log l) -> result<Out> { 
          return std::unexpected(failure{failure::error, l});  // error failure
        }
      );
    }

    template<std::ranges::contiguous_range Rng>
    constexpr result<Out> operator()(Rng const& rng, State init = {}) const {
      return (*this)(std::cbegin(rng), std::cend(rng), std::move(init));
    }

    P run;
  };

  template<typename In, typename State, typename Out, typename P>
  constexpr auto make_parser(P p) {
    return parser_t<In, State, Out, P>{ p };
  }

  //!
  //! Erased interface for parsers hiding the underlying closure type.
  //!
  template<typename In, typename State, typename Out>
  class erased_parser_t
  {
  public:
    using closure_t = result<Out>(const In *, const In *, State);

    template<typename P>
    erased_parser_t(P p) 
      : _parser{[=](const In *begin, const In *end, State state) {
        return make_parser<In, State, Out>(p)(begin, end, state);
      }} { }

    auto operator()(auto state, auto succ, auto fail, auto err) const {
      result<Out> res = _parser(
        std::to_address(state.begin), std::to_address(state.end), 
        std::move(state.state)
      );

      if(res.has_value())
        return succ(res->value, std::move(state));
      
      failure f = res.error();
      if(f.kind == failure::empty)
        return fail(f.log);
      
      return err(f.log);
    }

  private:
    std::function<closure_t> _parser;
  };

  template<typename In, typename State, typename Out>
  using parser = parser_t<In, State, Out, erased_parser_t<In, State, Out>>;

  template<typename In, typename State, typename Out, typename P>
  parser<In, State, Out> erased(parser_t<In, State, Out, P> p) {
    return parser<In, State, Out>{p};
  }

  //!
  //! Alwasys succeeds returning the given value.
  //!
  template<typename In = char, typename State = std::monostate, typename Out>
  inline constexpr auto succeed(Out out) {
    return make_parser<In, State, Out>(
      [out = std::move(out)](auto state, auto succ, auto&&, auto&&) {
        return succ(std::move(out), std::move(state));
      }
    );
  }

  //!
  //! Always fails.
  //!
  template<
    typename In = char, typename State = std::monostate, 
    typename Out = std::monostate
  >
  inline constexpr auto fail() {
    return make_parser<In, State, Out>(
      [=](auto&&, auto&&, auto f, auto&&) {
        return f({});
      }
    );
  }

  //!
  //! Always errors.
  //!
  template<
    typename In = char, typename State = std::monostate, 
    typename Out = std::monostate
  >
  inline constexpr auto error(std::string err) {
    return make_parser<In, State, Out>(
      [=](auto state, auto&&, auto&&, auto e) {
        state.log.errors.push_back(err);
        return e(std::move(state.log));
      }
    );
  }

  //!
  //! Matches anything if not at EOF, consumes it, and returns it.
  //!
  template<typename In = char, typename State = std::monostate>
  inline constexpr auto anything() {
    return make_parser<In, State, In>(
      [](auto state, auto succ, auto fail, auto&&) {
        if(state.begin == state.end)
          return fail(std::move(state.log));
        return succ(std::move(*state.begin++), std::move(state));
      }
    );
  }

  //!
  //! Matches the first of the two that succeeds, goes to the second if the
  //! first fails, errors if the first errors. Returns the result of the
  //! matching parser.
  //!
  template<typename In, typename State, typename Out, typename P1, typename P2>
  constexpr auto 
  either(parser_t<In, State, Out, P1> p1, parser_t<In, State, Out, P2> p2) {
    return make_parser<In, State, Out>(
      [=](auto state, auto succ, auto fail, auto err) {
        auto second = [=](log l) mutable {
          state.append(l);
          return p2.run(std::move(state), succ, fail, err);
        };
        return p1.run(std::move(state), succ, std::move(second), err);
      }
    );
  }

  //!
  //! Matches the given parser and gives the result to the given function, which
  //! must produce a parser again.
  //!
  template<typename In, typename State, typename Out, typename P, typename F>
  constexpr auto bind(parser_t<In, State, Out, P> p, F f) {
    using Res = typename std::invoke_result_t<F, Out>::output_type;

    return make_parser<In, State, Res>(
      [=](auto state1, auto succ, auto fail, auto err) {
        auto ok = [=](Out out, auto state2) {
          return f(std::move(out)).run(std::move(state2), succ, fail, err);
        };
        return p.run(std::move(state1), std::move(ok), fail, err);
      }
    );
  }

  //!
  //! Constructs a parser that lazily build another one by callling a lambda.
  //!
  template<typename F>
  constexpr auto recursive(F f) {
    using P     = std::invoke_result_t<F>;
    using In    = typename P::input_type;
    using State = typename P::state_type;
    using Out   = typename P::output_type;

    return make_parser<In, State, Out>(
      [=](auto state, auto succ, auto fail, auto err) {
        return f().run(std::move(state), succ, fail, err);
      }
    );
  }

  template<typename F, typename T>
  struct is_applicable : std::false_type { };

  template<typename F, typename T>
  inline constexpr bool is_applicable_v = is_applicable<F, T>::value;

  template<typename F, typename ...Ts>
  struct is_applicable<F, std::tuple<Ts...>> : std::is_invocable<F, Ts...> { };

  //!
  //! Matches the given parser and maps the given function to the returned
  //! value.
  //!
  template<typename In, typename State, typename Out, typename P, typename F>
  constexpr auto apply(parser_t<In, State, Out, P> p, F f) {
    return bind(
      std::move(p), [=](Out x) {
        if constexpr( is_applicable_v<F, Out> )
          return succeed(std::apply(f, std::move(x)));
        else
          return succeed(f(std::move(x))); 
      }
    );
  }

  //!
  //! Matches the given parsers in sequence returning the result of the second.
  //!
  constexpr auto seq(auto p1, auto p2) {
    return bind(std::move(p1), [=](auto&&) { return p2; });
  }

  constexpr auto seq(auto p1, auto p2, auto p3, auto ...ps) {
    return seq(p1, seq(p2, p3, ps...));
  }

  //!
  //! Matches the given parsers in sequence and collects all the results in a
  //! tuple.
  //!
  constexpr auto collect(auto p1, auto p2) {
    return bind(p1, [=](auto x) {
      return apply(p2, [x = std::move(x)]<typename Out2>(Out2 y) {
        if constexpr( requires {std::tuple_cat(std::tuple{x}, y);} )
          return std::tuple_cat(std::tuple{std::move(x)}, std::move(y));
        else
          return std::tuple{x,y};
      });
    });
  }

  constexpr auto collect(auto p1, auto p2, auto p3, auto ...ps) {
    return collect(p1, collect(p2, p3, ps...));
  }

  //!
  //! Matches any element that satisfies the given predicate and returns it.
  //!
  template<typename In = char, typename State = std::monostate, typename F>
  constexpr auto satisfies(F pred) {
    return bind(anything<In, State>(), [=](In x) -> parser<In, State, In> {
      if(pred(x))
        return succeed<In, State>(x);
      return fail<In, State, In>();
    });
  }

  //!
  //! Matches the single given element and returns it.
  //!
  template<typename State = std::monostate, typename In>
  inline constexpr auto element(In elem) {
    return satisfies<In, State>([=](In x) { return x == elem; });
  }

  //!
  //! Matches a parser and if it fails succeds returning a default value
  //!
  template<typename In, typename State, typename Out, typename P>
  constexpr auto maybe(Out x, parser_t<In, State, Out, P> p) {
    return either(p, succeed<In, State>(x));
  }

  //!
  //! Matches a parser and returns an `std::optional` which is empty if the
  //! parser fails.
  //!
  template<typename In, typename State, typename Out, typename P>
  constexpr auto optional(parser_t<In, State, Out, P> p) {
    return either(
      apply(p, [](Out x) { return std::optional<Out>{x}; }), 
      succeed<In, State>(std::optional<Out>{})
    );
  }

  //!
  //! Matches zero or more occurrences of the given parser and returns an
  //! `std::deque` of the return values.
  //!
  template<typename In, typename State, typename Out, typename P>
  constexpr 
  parser<In, State, std::deque<Out>> some(parser_t<In, State, Out, P> p) {
    return apply(
      collect(p, maybe(std::deque<Out>{}, recursive([=]{ return many(p); }))),
      [](Out out, std::deque<Out> outs) {
        outs.push_front(out);
        return outs;
      }
    );
  }
  
  //!
  //! Matches zero or more occurrences of the given parser and returns an
  //! `std::deque` of the return values.
  //!
  template<typename In, typename State, typename Out, typename P>
  constexpr auto many(parser_t<In, State, Out, P> p) {
    return maybe(std::deque<Out>{}, some(p));
  }

}

#endif // BLACK_INTERNAL_PARSING_COMBINATORS_HPP
