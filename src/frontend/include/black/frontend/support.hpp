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

#include <string>
#include <type_traits>
#include <cstdio>
#include <cstring>

namespace black::frontend
{
  //
  // Returns the string representation of a system error in a portable way
  //
  inline std::string system_error_string(int errnum)
  {
    char buf[255];
    const int buflen = sizeof(buf);

    // strerror has different return types on Linux and MacOS
#ifdef _GNU_SOURCE
    // GNU-specific version
    return strerror_r(errnum, buf, buflen);
#else
    // XSI-compliant systems, including MacOS
    strerror_r(errnum, buf, buflen);
    return buf;
#endif
  }
}

#endif // BLACK_FRONTEND_SUPPORT_HPP
