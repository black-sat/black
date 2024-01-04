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

#ifndef BLACK_SUPPORT_EXCEPTIONS_HPP
#define BLACK_SUPPORT_EXCEPTIONS_HPP

#include <black/support/config.hpp>

#include <stdexcept>
#include <string>
#include <string_view>
#include <format>
#include <version>
#include <typeinfo>
#include <source_location>

#include <cstring>
#include <cstdlib>

//
// This file declares the exception types used throught BLACK
//
namespace black::support::internal {

  //
  // The error exception type is the base class of all the exceptions
  // thrown from within BLACK
  //
  class exception : public std::logic_error 
  {
  public:
    exception(const char *what = "you should never see this error") 
      : logic_error(what) { }
    
    virtual ~exception() override = default;

    virtual const char *what() const noexcept override { 
      return _what.c_str(); 
    }

  protected:
    std::string _what;
  };

  inline const char *relative(const char *path) 
  {
    const char *p = path;
    const char *bp = BLACK_SOURCE_PATH;
    while(*bp != 0 && *path != 0) {
      char c = *p == '\\' ? '/' : (char)tolower(*p);
      char bc = *bp == '\\' ? '/' : (char)tolower(*bp);
      if(bc != c)
        return path;
      bp++;
      p++;
    }

    if(*bp == 0)
      return p;

    return path;
  }

  //
  // exception thrown on failure of `black_unreachable()`
  //
  class bad_unreachable : public exception 
  {
  public:
    bad_unreachable(const char *filename, size_t line) 
      : _filename{relative(filename)}, _line{line} 
    { 
      _what = std::format(
        "unreachable code reached at {}:{}", filename, line
      );
    }
    virtual ~bad_unreachable() override = default;

    std::string_view filename() const { return _filename; }
    size_t line() const { return _line; }

  private:
    const char *_filename = nullptr;
    size_t _line = 0;
  };

  //
  // exception thrown on failure of `black_assert()`
  //
  class bad_assert : public exception 
  {
  public:
    bad_assert(
      const char *filename, size_t line, const char *expression
    ) : _filename{relative(filename)}, _line{line}, _expression{expression}
    { 
      _what = std::format(
        "failed assertion at {}:{}: {}", filename, line, expression
      );
    }
    virtual ~bad_assert() override = default;

    std::string_view filename() const { return _filename; }
    size_t line() const { return _line; }
    std::string_view expression() const { return _expression; }

  private:
    const char *_filename = nullptr;
    size_t _line = 0;
    const char *_expression = nullptr;
  };

  //
  // exception thrown on failure of `black_assume()`
  //
  class bad_assumption : public bad_assert 
  {
  public:
    bad_assumption(
      const char *function, 
      const char *filename, size_t line, 
      std::source_location const& loc,
      const char *expression, std::string_view message,
      std::format_args const&args
    ) : bad_assert(relative(filename), line, expression), 
        _function{function}, _message{std::vformat(message, args)}
    { 
      if(loc.file_name() != nullptr) {
        filename = relative(loc.file_name());
        line = loc.line();
      }

      _what = std::format(
        "violated assumption when calling function '{}' at {}:{}: {}",
        function, filename, line, _message
      );
    }

    virtual ~bad_assumption() override = default;

    std::string_view function() const { return _function; }
    std::string_view message() const { return _message; }

  private:
    const char *_function = nullptr;
    std::string _message;
  };

  //
  // Exceptions thrown on non-exhaustive pattern matches.
  // The first constructor parameter is meant to be the object being matched,
  // whose type is the one missing in the list of handlers of match().
  //
  class bad_pattern : public exception
  {
  public:
    template<typename Case>
    bad_pattern(Case const&, std::source_location loc) { 
      _what = std::format(
        "non-exhaustive pattern match: missing handler for type `{}` "
        "at {} line {}", type_name<Case>(), 
        relative(loc.file_name()), loc.line()
      );
    }

    virtual ~bad_pattern() override = default;
  };

}

namespace black::support {
  using internal::exception;
  using internal::bad_unreachable;
  using internal::bad_assert;
  using internal::bad_assumption;
  using internal::bad_pattern;
}

#endif // BLACK_SUPPORT_EXCEPTIONS_HPP
