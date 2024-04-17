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
  struct success {
    T value;
    struct log log;
  };

  struct failure {
    enum { empty, error } kind;
    struct log log;
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
        [](Out const& out, auto state) -> result<Out> { 
          return success{out, std::move(state.log)}; // success
        },
        [](log l) -> result<Out> { 
          return std::unexpected(failure{failure::empty, l});  // empty failure
        }, 
        [](log l) -> result<Out> { 
          return std::unexpected(failure{failure::error, l});  // error failure
        },
        []{ black_unreachable(); }
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

  template<typename St, typename Out, typename Result>
  using success_closure_t = Result(Out, St);
  
  template<typename Result>
  using fail_err_closure_t = Result(log);
  
  template<typename St, typename Out, typename Result>
  using erased_closure_t = 
    std::function<
      Result(
        St, std::function<success_closure_t<St, Out, Result>>, 
        std::function<fail_err_closure_t<Result>>, 
        std::function<fail_err_closure_t<Result>>
      )
    >;

  //!
  //! Parser with an erased internal closure.
  //!
  template<typename In, typename State, typename Out>
  class erased_parser_t
  {
    using erased_state_t = state_t<const In *, const In *, State>;
    using closure_t = erased_closure_t<erased_state_t, Out, std::any>;

  public:
    template<typename P>
    erased_parser_t(P parser) 
      : _closure{[=](auto state, auto succ, auto fail, auto err) {
        return parser(std::move(state), succ, fail, err, [](auto&&...) {
          black_unreachable();
        });
      }} { }

    template<typename St, typename S, typename F, typename E, typename R>
    auto operator()(St state, S succ, F fail, E err, R) const {
      using Ret = std::invoke_result_t<S, Out, St>;
      
      erased_state_t st = { 
        std::to_address(state.begin),
        std::to_address(state.end),
        std::move(state.state),
        std::move(state.log)
      };

      auto result = _closure(std::move(st), succ, fail, err);
      black_assert(any_cast<Ret>(&result) != nullptr);

      return any_cast<Ret>(std::move(result));
    }
  
  private:
    closure_t _closure;
  };

  template<typename In, typename State, typename Out>
  using parser = parser_t<In, State, Out, erased_parser_t<In, State, Out>>;

  //!
  //! Alwasys succeeds returning the given value.
  //!
  template<typename In = char, typename State = std::monostate, typename Out>
  inline constexpr auto succeed(Out out) {
    return make_parser<In, State, Out>(
      [=](auto state, auto succ, auto, auto, auto) {
        return succ(out, std::move(state));
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
      [=](auto&&, auto&&, auto f, auto, auto) {
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
      [=](auto state, auto&&, auto&&, auto e, auto) {
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
      [](auto state, auto succ, auto fail, auto, auto) {
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
      [=](auto state, auto succ, auto fail, auto err, auto rec) {
        auto second = [=](log l) {
          auto st = state;
          st.append(l);
          return p2.run(std::move(st), succ, fail, err, rec);
        };
        return p1.run(std::move(state), succ, std::move(second), err, rec);
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
      [=](auto state1, auto succ, auto fail, auto err, auto rec) {
        auto ok = [=](auto const& out, auto state2) {
          return f(out).run(std::move(state2), succ, fail, err, rec);
        };
        return p.run(std::move(state1), std::move(ok), fail, err, rec);
      }
    );
  }

  //!
  //! Placeholder representing the recursive parser being built inside a
  //! `recursive()` call.
  //!
  template<typename In, typename State, typename Out>
  inline constexpr auto self() {
    return make_parser<In, State, Out>(
      []<typename St, typename S, typename F, typename E, typename R>
      (St state, S succ, F fail, E err, R rec) {
        return rec(std::move(state), succ, fail, err);
      }
    );
  }

  template<typename In, typename State, typename Out, typename P>
  struct recursion_t {

    recursion_t(parser_t<In, State, Out, P> parser) : _parser{parser} { }

    template<
      typename St, typename S, typename F, typename E,
      typename Ret = std::invoke_result_t<S, Out, St>
    >
    Ret operator()(St state, S succ, F fail, E err) const {
      using Closure = erased_closure_t<St, Out, Ret>;
      return _parser.run(std::move(state), succ, fail, err, Closure{*this});
    }

    parser_t<In, State, Out, P> _parser;
  };

  //!
  //! Builds a recursive parser by accepting any other parser built using the
  //! `self()` helper function to represent a recursive call to iself.
  //!
  template<typename In, typename State, typename Out, typename P>
  constexpr auto recursive(parser_t<In, State, Out, P> p) {
    return make_parser<In, State, Out>( 
      [=](auto state, auto succ, auto fail, auto err, auto) {
        return p.run(std::move(state), succ, fail, err, recursion_t{ p });
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
    return bind(p, [=](Out const& out) {
      if constexpr( is_applicable_v<F, Out> )
        return succeed(std::apply(f, out));
      else
        return succeed(f(out)); 
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
  //! TODO: check if copies of output values are avoided correctly.
  //!
  constexpr auto collect(auto p1, auto p2) {
    return bind(p1, [=]<typename Out1>(Out1 const& out1) {
      return apply(p2, [&](auto out2) {
        if constexpr( requires {std::tuple_cat(std::tuple{out1}, out2);} )
          return std::tuple_cat(std::tuple<Out1 const&>{out1}, out2);
        else
          return std::tuple{out1, out2};
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
    return make_parser<In, State, In>(
      [=](auto state, auto succ, auto fail, auto, auto) {
        if(state.begin == state.end || !pred(*state.begin))
          return fail(std::move(state.log));
        return succ(std::move(*state.begin++), std::move(state));
      }
    );
  }

  //!
  //! Matches the single given element and returns it.
  //!
  template<typename State = std::monostate, typename In>
  inline constexpr auto element(In elem) {
    return satisfies<In, State>([=](In x) { return x == elem; });
  }

  //!
  //! Matches the given parser and returns the object of the given type
  //! constructed from the return type.
  //!
  template<typename To, typename In, typename State, typename Out, typename P>
  constexpr auto to(parser_t<In, State, Out, P> p) {
    return apply(p, [](auto const& ...vs) { return To{vs...}; });
  }
  
  template<
    template<typename ...> class To, 
    typename In, typename State, typename Out, typename P
  >
  constexpr auto to(parser_t<In, State, Out, P> p) {
    return apply(p, [](auto const& ...vs) { return To{vs...}; });
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
      to<std::optional>(p), 
      succeed<In, State>(std::optional<Out>{})
    );
  }

  //!
  //! Matches zero or more occurrences of the given parser and returns an
  //! `std::deque` of the return values.
  //!
  template<
    typename In, typename State, typename Out, typename P,
    typename Res = std::deque<Out>
  >
  constexpr auto some(parser_t<In, State, Out, P> p) {
    return recursive(
      apply(
        collect(p, maybe(Res{}, self<In, State, Res>())),
        [](Out out, Res outs) {
          outs.push_front(std::move(out));
          return outs;
        }
      )
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
