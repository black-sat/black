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

namespace black::support {
  
  //! (Non-polymorphic) base class for all the following error classes.
  struct error 
  {
    //! \param format A format string (see `std::format`) for the error message.
    //! \param args The format string arguments.
    template<typename ...Args>
    error(const char *format, Args const& ...args)
      : message{
        std::vformat(format, std::make_format_args(args...))
      } { }

    error(error const&) = default;
    error(error &&) = default;
    
    error &operator=(error const&) = default;
    error &operator=(error &&) = default;

    bool operator==(error const&) const = default;
    
    std::string message; //!< The error message.
  };

  //! A syntax error occurred in any input file read by BLACK
  struct syntax_error : error 
  {
    //! \param filename The filename (see `filename` below).
    //! \param line The error line.
    //! \param column The error column.
    //! \param format A format string (see `std::format`) for the error message.
    //! \param args The format string arguments.
    template<typename ...Args>
    syntax_error(
      std::optional<std::string> filename, size_t line, size_t column,
      const char *format, Args const& ...args
    ) : error{format, args...}, 
        filename{filename}, line{line}, column{column} { }

    //! The name of the file where the syntax error occurred.
    //! If `filename` is `std::nullopt`, the error came from some source
    //! different from a file (*e.g.* the standard input stream).
    std::optional<std::string> filename; 

    size_t line = 0; //!< The line where the syntax error occurred.

    size_t column = 0; //!< The column where the syntax error occurred.
  };

  //! An error occurred while reading or writing from files or streams
  struct io_error : error {

    //! Kind of operation that caused the error
    enum operation {
      opening, //!< The operation was the opening of a file
      reading, //!< The operation was a read from a file or stream
      writing  //!< The operation was a write to a file or stream
    };

    //! \param filename The filename (see `filename` below). 
    //! \param op The kind of operation that caused the error. 
    //! \param err The value of the `errno` variable describing the error.
    //! \param format A format string (see `std::format`) for the error 
    //!               message. 
    //! \param args The format string arguments.
    template<typename ...Args>
    io_error(
      std::optional<std::string> filename, operation op, int err, 
      const char *format, Args const& ...args
    ) : error{format, args...}, 
        filename{filename}, op{op}, err{err} { }

    //! The name of the file on which the error occurred.
    //! If `filename` is `std::nullopt`, the error came from some source.
    //! different from a file (*e.g.* the standard input stream).
    std::optional<std::string> filename;
    operation op; //!< The kind of operation that caused the error.
    int err; //!< The value of the `errno` variable describing the error.
  };  

  //! A small function to unwrap std::optional/std::expected.
  //! Basically a safe version of operator*
  template<typename T>
  T const& unwrap(
    std::optional<T> const& opt, 
    std::source_location loc = std::source_location::current()
  ) {
    black_assume(opt.has_value(), loc, "tried to unwrap an empty optional");
    return *opt;
  }

  template<typename T>
  T && unwrap(
    std::optional<T> && opt, 
    std::source_location loc = std::source_location::current()
  ) {
    black_assume(opt.has_value(), loc, "tried to unwrap an empty optional");
    return *std::move(opt);
  }
  
  template<typename T, typename E>
  T & unwrap(
    std::expected<T, E> const& exp, 
    std::source_location loc = std::source_location::current()
  ) {
    black_assume(exp.has_value(), loc, "tried to unwrap an invalid expected");
    return *exp;
  }
  
  template<typename T, typename E>
  T && unwrap(
    std::expected<T, E> && exp, 
    std::source_location loc = std::source_location::current()
  ) {
    black_assume(exp.has_value(), loc, "tried to unwrap an invalid expected");
    return *std::move(exp);
  }

}


#endif // BLACK_SUPPORT_ERRORS_HPP
