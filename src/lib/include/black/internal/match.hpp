//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#ifndef BLACK_LOGIC_MATCH_HPP_
#define BLACK_LOGIC_MATCH_HPP_

#include <black/support/meta.hpp>
#include <functional>
#include <type_traits>

namespace black::internal
{
  using std::get;
  /*
   * Dummy type for default case in matchers
   */
  struct otherwise {};

  // this is just like std::apply but applies the formula f before the args
  template<typename Handler, typename Formula, size_t ...I>
  auto unpack_(
    Handler&& handler, Formula f, std::index_sequence<I...>
  ) -> RETURNS_DECLTYPE(FWD(handler)(f, get<I>(f)...))

  template<typename Handler, typename Formula>
  auto unpack(Handler&& handler, Formula f)
  -> RETURNS_DECLTYPE(
    unpack_(
      FWD(handler), f, std::make_index_sequence<std::tuple_size_v<Formula>>{}
    )
  )

  template<typename Handler, typename Formula, typename = void>
  struct can_be_unpacked : std::false_type { };

  template<typename Handler, typename Formula>
  struct can_be_unpacked<
    Handler, Formula, 
    std::void_t<
      decltype(
        unpack(std::declval<Handler>(), std::declval<Formula>())
      )
    >
  > : std::true_type { };
  
  //
  // The dispatch() function is what does the hard job
  //
  template<
    typename Formula, typename Handler, typename ... Handlers,
    REQUIRES(std::is_invocable_v<Handler, Formula>)
  >
  auto dispatch(Formula f, Handler&& handler, Handlers&& ...) {
    return std::invoke(FWD(handler), f);
  }

  template<
    typename Formula, typename Handler, typename ...Handlers,
    REQUIRES(!std::is_invocable_v<Handler, Formula>),
    REQUIRES(can_be_unpacked<Handler, Formula>::value)
  >
  auto dispatch(Formula f, Handler&& handler, Handlers&& ...) {
    return unpack(FWD(handler), f);
  }

  template<
    typename Formula, typename H1, typename H2, typename ...Handlers,
    REQUIRES(!std::is_invocable_v<H1, Formula>),
    REQUIRES(!can_be_unpacked<H1, Formula>::value)
  >
  auto dispatch(Formula f, H1&&, H2&& h2, Handlers&& ...handlers) 
    -> decltype(dispatch(f, FWD(h2), FWD(handlers)...)) 
  {
    return dispatch(f, FWD(h2), FWD(handlers)...);
  }

  template<typename ...Operators>
  struct syntax { };

  template<typename Formula, typename ...Cases>
  struct matcher;

  template<typename Formula, typename Case>
  struct matcher<Formula, syntax<Case>> {
    template<typename ...Handlers>
    static auto match(Formula f, Handlers&& ...handlers)
      -> decltype(dispatch(*f.template to<Case>(), FWD(handlers)...))
    {
      if(f.template is<Case>())
        return dispatch(*f.template to<Case>(), FWD(handlers)...);
      
      black_unreachable(); // LCOV_EXCL_LINE
    }
  };

  template<typename Formula, typename Case, typename ...Cases>
  struct matcher<Formula, syntax<Case, Cases...>>
  {
    template<typename ...Handlers>
    static auto match(Formula f, Handlers&& ...handlers) 
      -> std::common_type_t<
        decltype(dispatch(*f.template to<Case>(), FWD(handlers)...)),
        decltype(matcher<Formula, syntax<Cases...>>::match(f, FWD(handlers)...))
      >
    {
      if(f.template is<Case>())
        return dispatch(*f.template to<Case>(), FWD(handlers)...);
      else
        return matcher<Formula, syntax<Cases...>>::match(f, FWD(handlers)...);
    }
  };
}

#endif
