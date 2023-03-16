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

#include <stdexcept>

//
// This file declares the exception types used throught BLACK
//
namespace black::support::internal {
  
  //
  // The error exception type is the base class of all the exceptions
  // thrown from within BLACK
  //
  class error : public std::logic_error 
  {
  public:
    error() : logic_error("you should never see this error") { }
    
    virtual ~error() override = default;

  };

  //
  // exception thrown on failure of `black_unreachable()`
  //
  class unreachable_error : public error 
  {
  public:
    unreachable_error(const char *filename, size_t line) 
      : _filename{filename}, _line{line} 
    { 
      std::snprintf(
        _what, 200, "unreachable code reached at %s:%zd",
        filename, line
      );
    }
    virtual ~unreachable_error() override = default;

    virtual const char *what() const noexcept override { return _what; }

    const char *filename() const { return _filename; }
    size_t line() const { return _line; }

  private:
    char _what[200];
    const char *_filename = nullptr;
    size_t _line = 0;
  };

  //
  // exception thrown on failure of `black_assert()`
  //
  class assert_error : public error 
  {
  public:
    assert_error(
      const char *filename, size_t line, const char *expression
    ) : _filename{filename}, _line{line}, _expression{expression}
    { 
      std::snprintf(
        _what, 200, "failed assertion at %s:%zd: \"%s\"",
        filename, line, expression
      );
    }
    virtual ~assert_error() override = default;

    virtual const char *what() const noexcept override { return _what; }

    const char *filename() const { return _filename; }
    size_t line() const { return _line; }
    const char *expression() const { return _expression; }

  protected:
    char _what[200];
  private:
    const char *_filename = nullptr;
    size_t _line = 0;
    const char *_expression = nullptr;
  };
  
  //
  // exception thrown on failure of `black_assume()`
  //
  class assume_error : public assert_error 
  {
  public:
    assume_error(
      const char *function, 
      const char *filename, size_t line, 
      const char *expression, const char *message
    ) : assert_error(filename, line, expression), 
        _function{function}, _message{message}
    { 
      std::snprintf(
        _what, 200, "violated assumption calling function '%s': \"%s\"",
        function, message
      );
    }
    virtual ~assume_error() override = default;

    const char *function() const { return _message; }
    const char *message() const { return _message; }

  private:
    const char *_function = nullptr;
    const char *_message = nullptr;
  };

}
