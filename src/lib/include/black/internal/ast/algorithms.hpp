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

#ifndef BLACK_INTERNAL_AST_ALGORITHMS_HPP
#define BLACK_INTERNAL_AST_ALGORITHMS_HPP

#include <numeric>

namespace black::ast {

  template<typename F>
  auto ignore(F f) {
    return 
      [=]<typename ...Args>(auto&&, Args ...args) 
        requires std::invocable<F, Args...>
      {
        return f(std::move(args)...);
      };
  }

  template<typename R, typename T, typename ...Fs>
  decltype(auto) traverse_(T const& v, Fs ...fs);

  template<typename R, core::ast AST, typename ...Fs>
  std::vector<R> traverse_(std::vector<AST> const& ts, Fs ...fs);

  template<typename R, core::ast AST, typename ...Fs>
  R traverse_(AST const& t, Fs ...fs) {
    using namespace black::support;

    return match(t)( 
      [=](auto n, auto ...args) -> R {
        return dispatch(
          std::make_tuple(
            n, traverse_<R>(args, fs...)...
          ), 
          unpacking(ignore(fs))...
        );
      }
    );
  }

  template<typename R, core::ast AST, typename ...Fs>
  std::vector<R> traverse_(std::vector<AST> const& ts, Fs ...fs) {
    std::vector<R> result;
    for(auto t : ts)
      result.push_back(traverse_<R>(t, fs...));
    return result;
  }

  template<typename R, typename T, typename ...Fs>
  decltype(auto) traverse_(T const& v, Fs ...) { return v; }

  template<typename R, core::ast AST>
  auto traverse(AST t) { 
    return [=](auto ...fs) {
      return traverse_<R>(t, fs...); 
    };
  }

  template<core::ast AST>
  auto map(AST t) { 
    return [=](auto ...fs) {
      return traverse_<AST>(
        t, fs..., []<typename N>(N, auto ...args) {
          return N(std::move(args)...);
        }); 
    };
  }

  template<typename R, typename ...Fs>
  auto traversal(Fs ...fs) {
    return [=](core::ast auto t) {
      return traverse_<R>(t, fs...);
    };
  }

  template<typename ...Fs>
  auto mapping(Fs ...fs) {
    return [=](core::ast auto t) {
      return map(t, fs...);
    };
  }

}

#endif // BLACK_INTERNAL_AST_ALGORITHMS_HPP
