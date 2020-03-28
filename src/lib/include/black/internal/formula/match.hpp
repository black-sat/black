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

#ifndef BLACK_LOGIC_FORMULA_HPP_
  #error "This header file cannot be included alone, "\
         "please include <black/logic/formula.hpp> instead"
#endif

namespace black::internal
{
  // First-match-first-called apply function
  template<typename ...Args, typename F, typename ...Fs>
  auto apply_first(std::tuple<Args...> args, F f, Fs ...fs)
  {
    if constexpr(std::is_invocable_v<F, Args...>) {
      return std::apply(f, args);
    } else if constexpr(sizeof...(Fs) > 0)
      return apply_first(args, fs...);
  }

  //
  // Implementation of the matching function, formula::match()
  //
  template<typename Case, typename ...Cases, typename ...Handlers>
  auto match(formula f, Handlers&& ...handlers) 
  {
    if(f.is<Case>())
      return apply_first(std::make_tuple(*f.to<Case>()), FWD(handlers)...);
    else if constexpr(sizeof...(Cases) > 0)
      return match<Cases...>(f, FWD(handlers)...);
    
    black_unreachable();
  }
  
  template<typename ...Handlers>
  auto formula::match(Handlers&& ...handlers) const {
    return internal::match<
      boolean,
      atom,
      negation,
      tomorrow,
      yesterday,
      always,
      eventually,
      past,
      historically,
      conjunction,
      disjunction,
      then,
      iff,
      until,
      release,
      since,
      triggered
    >(*this, FWD(handlers)...);
  }
}

#endif // BLACK_LOGIC_MATCH_HPP_
