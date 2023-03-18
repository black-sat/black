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
#include <cstring>

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

    virtual const char *what() const noexcept override { return _what; }

  protected:
    char _what[200];
  };

  inline const char *relative(const char *path) {
    const char *rel = nullptr;
    if((rel = strstr(path, BLACK_SOURCE_PATH)) == path)
      return path + strlen(BLACK_SOURCE_PATH);
    return path;
  }

  //
  // exception thrown on failure of `black_unreachable()`
  //
  class unreachable_error : public exception 
  {
  public:
    unreachable_error(const char *filename, size_t line) 
      : _filename{relative(filename)}, _line{line} 
    { 
      std::snprintf(
        _what, 200, "unreachable code reached at %s:%zd",
        filename, line
      );
    }
    virtual ~unreachable_error() override = default;

    const char *filename() const { return _filename; }
    size_t line() const { return _line; }

  private:
    const char *_filename = nullptr;
    size_t _line = 0;
  };

  //
  // exception thrown on failure of `black_assert()`
  //
  class assert_error : public exception 
  {
  public:
    assert_error(
      const char *filename, size_t line, const char *expression
    ) : _filename{relative(filename)}, _line{line}, _expression{expression}
    { 
      std::snprintf(
        _what, 200, "failed assertion at %s:%zd: %s",
        filename, line, expression
      );
    }
    virtual ~assert_error() override = default;

    const char *filename() const { return _filename; }
    size_t line() const { return _line; }
    const char *expression() const { return _expression; }

  private:
    const char *_filename = nullptr;
    size_t _line = 0;
    const char *_expression = nullptr;
  };

  //
  // Work around for compilers not supporting std::source_location
  //  
  struct dummy_source_location 
  {
    consteval dummy_source_location() = default;
    dummy_source_location(dummy_source_location const&) = default;
    dummy_source_location(dummy_source_location &&) = default;

    static consteval dummy_source_location current() noexcept { 
      return dummy_source_location{};
    }

    constexpr std::uint_least32_t line() const noexcept { return 0; }
    constexpr std::uint_least32_t column() const noexcept { return 0; }
    constexpr const char* file_name() const noexcept { return nullptr; }
    constexpr const char* function_name() const noexcept { return nullptr; }
  };

  #ifdef __cpp_lib_source_location
    using source_location = std::source_location;
  #else
    using source_location = dummy_source_location;
  #endif 

  //
  // exception thrown on failure of `black_assume()`
  //
  class assume_error : public assert_error 
  {
  public:
    assume_error(
      const char *function, 
      const char *filename, size_t line, 
      source_location const& loc,
      const char *expression, const char *message
    ) : assert_error(relative(filename), line, expression), 
        _function{function}, _message{message}
    { 
      if(loc.file_name() != nullptr) {
        function = loc.function_name();
        filename = relative(loc.file_name());
        line = loc.line();
      }

      std::snprintf(
        _what, 200, 
        "violated assumption when calling function '%s' at %s:%zd: %s",
        function, filename, line, message
      );
    }
    virtual ~assume_error() override = default;

    const char *function() const { return _function; }
    const char *message() const { return _message; }

  private:
    const char *_function = nullptr;
    const char *_message = nullptr;
  };

  class pattern_error : public exception 
  {
  public:
    pattern_error() { 
      strncpy(_what, "non-exhaustive pattern", 200);
    }

    virtual ~pattern_error() override = default;

  private:
    source_location _loc;
  };

}

namespace black::support {
  using internal::exception;
  using internal::unreachable_error;
  using internal::assert_error;
  using internal::assume_error;
}

#endif // BLACK_SUPPORT_EXCEPTIONS_HPP
