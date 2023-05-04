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

#ifndef BLACK_SUPPORT_ERRORS_HPP
#define BLACK_SUPPORT_ERRORS_HPP

#include <fmt/format.h>

namespace black::support::internal {
  
  struct error_base {
    std::string message;

    template<typename ...Args>
    error_base(const char *format, Args&& ...args)
      : message{
        fmt::vformat(format, fmt::make_format_args(std::forward<Args>(args)...))
      } { }
  };

  struct syntax_error : error_base {
    std::optional<std::string> filename;
    size_t line = 0;
    size_t column = 0;

    template<typename ...Args>
    syntax_error(
      std::optional<std::string> _filename, size_t _line, size_t _col,
      const char *format, Args const& ...args
    ) : error_base{format, args...}, 
        filename{_filename}, line{_line}, column{_col} { }
  };

  struct type_error : error_base { 
    using error_base::error_base;
  };

  struct backend_error : error_base { 
    using error_base::error_base;
  };

  struct io_error : error_base {
    enum operation {
      opening,
      reading,
      writing
    };

    std::optional<std::string> filename;
    operation op;
    int error;

    template<typename ...Args>
    io_error(
      std::optional<std::string> _filename, operation _op, int _error,
      const char *format, Args const& ...args
    ) : error_base{format, args...}, 
        filename{_filename}, op{_op}, error{_error} { }
  };

  struct error : 
    black_sum_type(syntax_error, type_error, backend_error, io_error);
  
}


#endif // BLACK_SUPPORT_ERRORS_HPP
