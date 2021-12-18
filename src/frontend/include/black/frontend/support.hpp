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

#ifndef BLACK_FRONTEND_SUPPORT_HPP
#define BLACK_FRONTEND_SUPPORT_HPP

#include <black/logic/formula.hpp>

#include <black/frontend/cli.hpp>
#include <black/frontend/io.hpp>

#include <fstream>
#include <string>
#include <type_traits>
#include <cstdio>
#include <cstring>
#include <functional>

#if defined(_MSC_VER)
  #include <BaseTsd.h>
  using ssize_t = SSIZE_T;
#endif

namespace black::frontend
{
  // Returns the string representation of a system error in a portable way
  std::string system_error_string(int errnum);

  // Opens a file and quits with an error if the file cannot be open
  std::ifstream open_file(std::string const&path);

  // common handler for syntax errors in the whole frontend
  std::function<void(std::string)> 
  formula_syntax_error_handler(std::optional<std::string> const&path);

  // enum with features of formulas interesting for the frontend
  enum class feature_t : uint8_t {
    temporal = 1,
    past = 2,
    first_order = 4,
    quantifiers = 8,
    nextvar = 16
  };

  // this function is executed multiple times but gcov doesn't get it
  inline uint8_t operator&(uint8_t f1, feature_t f2) { // LCOV_EXCL_LINE
    return f1 & (uint8_t)f2; // LCOV_EXCL_LINE
  }

  // tells which features are present in a formula
  uint8_t formula_features(formula f);
}

#endif // BLACK_FRONTEND_SUPPORT_HPP
