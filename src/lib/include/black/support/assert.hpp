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

namespace black_internal {

  template<typename Expr>
  constexpr void assert_handler(
    Expr assertion, const char *expression, const char *filename, size_t line
  ) noexcept 
  {
    if(assertion())
      return;
    
    std::fprintf(stderr, "[black]: failed assert at %s:%zu, \"%s\"\n",
                 filename, line, expression);
    std::abort();
  }

  constexpr 
  void unreachable_handler(bool dummy, const char *filename, size_t line) 
  noexcept
  {
    if(dummy)
      return;
    
    std::fprintf(stderr, 
                 "[black]: unreachable code reached at %s:%zu\n",
                 filename, line);
    std::abort();
  }

}

#ifndef BLACK_ASSERT_DISABLE

  #define BLACK_ASSERT(Expr)                            \
    black_internal::assert_handler(                    \
      [&]() { return static_cast<bool>(Expr); }, #Expr, __FILE__, __LINE__ \
    )

  #define BLACK_UNREACHABLE()                                        \
    black_internal::unreachable_handler(false, __FILE__, __LINE__); \
    BLACK_MARK_UNREACHABLE

#else 
  
  #define BLACK_ASSERT(Expr)
  #define BLACK_UNREACHABLE() BLACK_MARK_UNREACHABLE

#endif

/*
 * borrowed from https://github.com/foonathan/debug_assert
 */
#ifndef BLACK_MARK_UNREACHABLE
# ifdef __GNUC__
#   define BLACK_MARK_UNREACHABLE __builtin_unreachable()
# elif defined(_MSC_VER)
#   define BLACK_MARK_UNREACHABLE __assume(0)
# else
#   define BLACK_MARK_UNREACHABLE
# endif
#endif

#ifndef BLACK_NO_LOWERCASE_MACROS
  #define black_assert BLACK_ASSERT
  #define black_unreachable BLACK_UNREACHABLE
#endif

#endif // BLACK_ASSERT_HPP
