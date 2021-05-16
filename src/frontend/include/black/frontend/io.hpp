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
  // io::print(format, args...)
  //
  template<typename... Args>
  auto print(Args&&... args)
    -> std::void_t<decltype(fmt::print(stdout, std::forward<Args>(args)...))>
  {
    fmt::print(stdout, std::forward<Args>(args)...);
  }

  //
  // println() is just as print(), but prints a newline
  //
  template<typename... Args>
  auto println(Args&&... args)
    -> std::void_t<decltype(fmt::print(std::forward<Args>(args)...))>
  {
    print(std::forward<Args>(args)...);
    print("\n");
  }

  //
  // io::error(format, args...)
  //
  template<typename... Args>
  auto error(Args&&... args)
    -> std::void_t<decltype(fmt::print(stderr, std::forward<Args>(args)...))>
  {
    fmt::print(stderr, std::forward<Args>(args)...);
  }

  //
  // io:errorln() is just as error(), but prints a newline
  //
  template<typename... Args>
  auto errorln(Args&&... args)
    -> std::void_t<decltype(error(std::forward<Args>(args)...))>
  {
    error(std::forward<Args>(args)...);
    error("\n");
  }


  //
  // fatal() is different, as it also quits the program,
  // and asks for the status code to return to the system
  //
  template<typename... Args>
  [[ noreturn ]]
  auto fatal(status_code v, Args&&... args)
    -> decltype(print(std::forward<Args>(args)...))
  {
    error("{}: ", cli::command_name);
    errorln(std::forward<Args>(args)...);

    quit(v);
  }
}

#endif // BLACK_FRONTEND_IO_HPP
