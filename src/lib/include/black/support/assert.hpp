//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

//
// Custom assert macros:
// - black_assert(Expr)
// - black_unreachable()
//

#ifndef BLACK_ASSERT_HPP
#define BLACK_ASSERT_HPP

#include <cstdio>
#include <cstdlib>

#include <black/support/exceptions.hpp>

namespace black::support::internal {

  template<typename Expr>
  constexpr void assert_handler(
    Expr assertion, const char *filename, size_t line, const char *expression
  )
  {
    if(assertion())
      return;
    
    throw bad_assert(filename, line, expression);
  }
  
  [[noreturn]]
  inline void unreachable_handler(const char *filename, size_t line) 
  {
    throw bad_unreachable(filename, line);
  }

  template<typename Expr>
  constexpr void assume_handler(
    Expr assumption, 
    const char *function,
    const char *filename, size_t line,
    std::source_location const& loc,
    const char *expression, const char *message
  )
  {
    if(assumption())
      return;
    
    throw bad_assumption(function, filename, line, loc, expression, message);
  }

}

#ifndef BLACK_ASSERT_DISABLE

  #define black_assert(Expr)                                               \
    black::support::internal::assert_handler(                              \
      [&]() { return static_cast<bool>(Expr); }, __FILE__, __LINE__, #Expr \
    )

#else 
  
  #define black_assert(Expr)

#endif

#define black_unreachable()                                            \
    black::support::internal::unreachable_handler(__FILE__, __LINE__); 


#define black_assume(Expr, Loc, Message)                \
  black::support::internal::assume_handler(             \
    [&]() { return static_cast<bool>(Expr); },          \
    __FUNCTION__, __FILE__, __LINE__, Loc, #Expr, Message \
  )

#endif // BLACK_ASSERT_HPP
