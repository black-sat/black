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

#ifndef BLACK_FRONTEND_IO_HPP
#define BLACK_FRONTEND_IO_HPP

#include <black/frontend/cli.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <unistd.h>

//
// Functions in this file should be used as the only way to output anything
// from the whole codebase, as they ensure consistent behavior across both the
// library and the frontend.
//
// For example:
// - They obey the global verbosity level
// - Messages with verbosity level <= warning are routed to stderr
// - Fatal errors are prepended with the program name
// - They switch off colored output if not attached to a terminal
//
namespace black::frontend::io
{
  //
  // Utility functions to test whether stdout or stderr are terminals
  //
  inline bool stdout_is_terminal() {
    return isatty(fileno(stdout));
  }

  inline bool stderr_is_terminal() {
    return isatty(fileno(stderr));
  }

  // We use <cstdio> from the C standard library to avoid to propagate the
  // inclusion of <iostream> to all the program.
  inline FILE *stream_for_verbosity(verbosity v) {
    return v > verbosity::warning ? stdout : stderr;
  }

  //
  // io::print(verbosity, format, args...)
  //
  template<typename... Args>
  auto print(verbosity v, Args&&... args)
    -> std::void_t<decltype(fmt::print(std::forward<Args>(args)...))>
  {
    FILE *file = stream_for_verbosity(v);

    if(v == verbosity::fatal)
      fmt::print(file, "{}: ", cli::command_name);

    fmt::print(file, std::forward<Args>(args)...);
  }

  //
  // println() is just as print(), but prints a newline
  //
  template<typename... Args>
  auto println(verbosity v, Args&&... args)
    -> std::void_t<decltype(fmt::print(std::forward<Args>(args)...))>
  {
    print(v, std::forward<Args>(args)...);
    fmt::print(stream_for_verbosity(v), "\n");
  }

  //
  // Specific versions of println() for each verbosity level
  //
  #define declare_verbose_print(Verbosity)                              \
    template<typename... Args>                                          \
    auto Verbosity(Args&& ...args)                                      \
      -> std::void_t<decltype(fmt::print(std::forward<Args>(args)...))> \
    {                                                                   \
      println(verbosity::Verbosity, std::forward<Args>(args)...);       \
    }

  declare_verbose_print(error)
  declare_verbose_print(warning)
  declare_verbose_print(message)
  declare_verbose_print(debug)
  declare_verbose_print(trace)

  #undef declare_verbose_print

  //
  // fatal() is different, as it also quits the program,
  // and asks for the status code to return to the system
  //
  template<typename... Args>
  [[ noreturn ]]
  auto fatal(status_code v, Args&&... args)
    -> decltype(fmt::print(std::forward<Args>(args)...))
  {
    println(verbosity::fatal, std::forward<Args>(args)...);

    quit(v);
  }
}

#endif // BLACK_FRONTEND_IO_HPP
