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

#include <black/ast/core>

namespace black::ast {

  template<typename F>
  auto wrap(F f) {
    return 
      [=]<typename N, typename ...Args>(auto&&, N n, Args ...args) 
        requires std::invocable<F, N, std::invoke_result_t<Args>...>
      {
        return f(n, args()...);
      };
  }

  template<typename R, core::ast AST, typename T, typename ...Fs>
    requires (!std::same_as<T, AST>)
  T const& traverse_(T const& v, std::source_location loc, Fs ...fs);

  template<typename R, core::ast AST, typename ...Fs>
  std::vector<R> 
  traverse_(std::vector<AST> const& ts, std::source_location loc, Fs ...fs);

  template<typename R, core::ast AST, typename ...Fs>
  R traverse_(AST const& t, std::source_location loc, Fs ...fs) {
    using namespace black::support;

    return match(t)( 
      [=](auto n, auto ...args) -> R {
        return dispatch(
          std::make_tuple(
            n, [=]{ return traverse_<R, AST>(args, loc, fs...); }... 
          ), 
          unpacking(wrap(fs))...,
          [&](auto missing) -> R { 
            throw bad_pattern(missing, loc); 
          }
        );
      }
    );
  }

  template<typename R, core::ast AST, typename ...Fs>
  std::vector<R> 
  traverse_(std::vector<AST> const& ts, std::source_location loc, Fs ...fs) {
    std::vector<R> result;
    for(auto t : ts)
      result.push_back(traverse_<R, AST>(t, loc, fs...));
    return result;
  }

  template<
    typename R, core::ast AST, typename T, typename ...Fs
  > requires (!std::same_as<T, AST>)
  T const& traverse_(T const& v, std::source_location, Fs ...) {   
    return v; 
  }

  template<typename R, core::ast AST>
  auto 
  traverse(AST t, std::source_location loc = std::source_location::current()) { 
    return [=](auto ...fs) {
      return traverse_<R, AST>(t, loc, fs...); 
    };
  }

  template<core::ast AST>
  auto map(AST t) { 
    return [=](auto ...fs) {
      return traverse<AST>(t)(
        fs..., []<typename N>(N, auto ...args) {
          return N(std::move(args)...);
        }); 
    };
  }

  template<typename R, typename ...Fs>
  auto traversal(Fs ...fs) {
    return [=]<core::ast AST>(
      AST t, std::source_location loc = std::source_location::current()
    ) {
      return traverse_<R, AST>(t, loc, fs...);
    };
  }

  template<typename ...Fs>
  auto mapping(Fs ...fs) {
    return [=](core::ast auto t) {
      return map(t)(fs...);
    };
  }

}

#endif // BLACK_INTERNAL_AST_ALGORITHMS_HPP
