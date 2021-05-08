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

#include <fstream>
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

  inline std::ifstream open_file(std::string const&path) {
    std::ifstream file{path, std::ios::in};

    if(!file)
      io::fatal(status_code::filesystem_error,
        "Unable to open file `{}`: {}",
        path, system_error_string(errno)
      );

    return file;
  }

  inline
  std::function<void(std::string)> 
  formula_syntax_error_handler(std::optional<std::string> const&path)
  {
    auto readable_syntax_error = [path](auto error) {
      io::fatal(status_code::syntax_error, 
                "syntax error: {}: {}\n", 
                path ? *path : "<stdin>", error);
    };

    auto json_syntax_error = [](auto error) {
      io::error(
        "{{\n"
        "    \"result\": \"ERROR\",\n"
        "    \"error\": \"{}\"\n"
        "}}", error);
      quit(status_code::syntax_error);
    };

    if(!cli::output_format || cli::output_format == "readable")
      return readable_syntax_error;
    
    return json_syntax_error;
  }
}

#endif // BLACK_FRONTEND_SUPPORT_HPP
