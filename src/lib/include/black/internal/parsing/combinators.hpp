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

namespace black::parsing {

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

  template<typename T>
  using result = std::expected<success<T>, failure>;

  template<typename In, typename State, typename Out>
  using success_cont_t = result<Out>(Out, State, log);

  template<typename Out>
  using empty_cont_t = result<Out>(log);

  template<typename Out>
  using error_cont_t = result<Out>(log);

  template<typename P, typename In, typename State, typename Out>
  concept parselet = 
    requires(
      P p, In *begin, In *end, State state,
      success_cont_t<In, State, Out> * s, 
      empty_cont_t<Out>              * e, 
      error_cont_t<Out>              * err
    ) {
      { p(begin, end, state, s, e, err) } -> std::same_as<result<Out>>;
    };

  template<
    typename In, typename State, typename Out, parselet<In, State, Out> P
  >
  struct parser_t {

    constexpr parser_t(P p) : parser{std::move(p)} { }

    template<std::ranges::forward_range Rng>
    result<Out> operator()(Rng const& rng, State init = {}) const {
      return parser(
        std::begin(rng), std::end(rng), std::move(init), 
        [](Out in, State, log l) { return success{in, l}; }, // success
        [](log l) { return std::unexpected(failure{l});   }, // empty failure
        [](log l) { return std::unexpected(failure{l});   }  // error failure
      );
    }

    P parser;
  };

  template<
    typename In, typename State, typename Out, parselet<In, State, Out> P
  >
  auto make_parser(P p) {
    return parser_t<In, State, Out, P>{ p };
  }


  template<typename In = char, typename State = std::monostate, typename Out>
  inline constexpr auto succeed(Out out) {
    return make_parser<In, State, Out>(
      [=](auto, auto, auto state, auto succ, auto, auto) {
        return succ(std::move(out), std::move(state), {});
      }
    );
  }

  template<
    typename In = char, typename State = std::monostate, 
    typename Out = std::monostate
  >
  inline constexpr auto fail() {
    return make_parser<In, State, Out>(
      [=](auto, auto, auto, auto, auto f, auto) {
        return f({});
      }
    );
  }

  template<
    typename In = char, typename State = std::monostate, 
    typename Out = std::monostate
  >
  inline constexpr auto error(std::string err) {
    return make_parser<In, State, Out>(
      [=](auto, auto, auto, auto, auto, auto e) {
        return e(log{{err}});
      }
    );
  }

}

#endif // BLACK_INTERNAL_PARSING_COMBINATORS_HPP
