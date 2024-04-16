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
    struct log log;
  };

  template<typename It, typename End, typename State>
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

  template<typename In, typename State, typename Out, typename P>
  struct parser_t {

    constexpr parser_t(P p) : parser{std::move(p)} { }

    template<std::ranges::forward_range Rng>
    constexpr result<Out> operator()(Rng const& rng, State init = {}) const 
    {
      using StateT = state_t<
        const_iterator_t<Rng>, const_sentinel_t<Rng>, State
      >;

      return parser(
        StateT{std::cbegin(rng), std::cend(rng), std::move(init), log{}}, 
        [](Out in, StateT state) -> result<Out> { 
          return success{in, std::move(state.log)};          // success
        },
        [](log l) -> result<Out> { 
          return std::unexpected(failure{l});                // empty failure
        }, 
        [](log l) { return std::unexpected(failure{l});   }  // error failure
      );
    }

    P parser;
  };

  template<typename In, typename State, typename Out, typename P>
  constexpr auto make_parser(P p) {
    return parser_t<In, State, Out, P>{ p };
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
  //! Matches the single given element and returns it.
  //!
  template<typename State = std::monostate, typename In>
  inline constexpr auto element(In elem) {
    return make_parser<In, State, In>(
      [elem = std::move(elem)]
      (auto state, auto succ, auto fail, auto&&) {
        if(state.begin == state.end || *state.begin != elem)
          return fail(std::move(state.log));
        state.begin++;
        return succ(std::move(elem), std::move(state));
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
          return p2.parser(std::move(state), succ, fail, err);
        };
        return p1.parser(std::move(state), succ, std::move(second), err);
      }
    );
  }

  //!
  //! Matches the given parser and gives the result to the given function, which
  //! must produce a parser again.
  //!
  template<typename In, typename State, typename Out, typename P, typename F>
  constexpr auto bind(parser_t<In, State, Out, P> p, F f) {
    return make_parser<In, State, Out>(
      [=](auto state1, auto succ, auto fail, auto err) {
        auto ok = [=](Out out, auto state2) {
          return f(std::move(out)).parser(std::move(state2), succ, fail, err);
        };
        return p.parser(std::move(state1), std::move(ok), fail, err);
      }
    );
  }

  //!
  //! Matches the given parser and maps the given function to the returned
  //! value.
  //!
  template<typename In, typename State, typename Out, typename P, typename F>
  constexpr auto apply(parser_t<In, State, Out, P> p, F f) {
    return bind(std::move(p), [f](auto x) { return succeed(f(std::move(x))); });
  }

  //!
  //! Matches the given parsers in sequence returning the result of the second.
  //!
  constexpr auto seq(auto p1, auto p2) {
    return bind(std::move(p1), [=](auto&&) { return p2; });
  }

  constexpr auto seq(auto p1, auto p2, auto p3, auto ...ps) {
    return seq(seq(p1, p2), p3, ps...);
  }

}

#endif // BLACK_INTERNAL_PARSING_COMBINATORS_HPP
