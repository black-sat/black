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

#include <format>

namespace black::support::internal {
  
  struct error {
    std::string message;

    template<typename ...Args>
    error(const char *format, Args const& ...args)
      : message{
        std::vformat(format, std::make_format_args(args...))
      } { }
  };

  struct syntax_error : error {
    std::optional<std::string> filename;
    size_t line = 0;
    size_t column = 0;

    template<typename ...Args>
    syntax_error(
      std::optional<std::string> _filename, size_t _line, size_t _col,
      const char *format, Args const& ...args
    ) : error{format, args...}, 
        filename{_filename}, line{_line}, column{_col} { }
  };

  struct type_error : error { 
    using error::error;
  };

  struct backend_error : error { 
    using error::error;
  };

  struct io_error : error {
    enum operation {
      opening,
      reading,
      writing
    };

    std::optional<std::string> filename;
    operation op;
    int err;

    template<typename ...Args>
    io_error(
      std::optional<std::string> _filename, operation _op, int _error,
      const char *format, Args const& ...args
    ) : error{format, args...}, 
        filename{_filename}, op{_op}, err{_error} { }
  };  
}

namespace black::support {
  using internal::error;
  using internal::syntax_error;
  using internal::type_error;
  using internal::backend_error;
  using internal::io_error;
}


#endif // BLACK_SUPPORT_ERRORS_HPP
