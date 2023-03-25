//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#include <black/support.hpp>


#include <fmt/format.h>
#include <fmt/color.h>

#include <cstdio>
#include <optional>

#ifdef _MSC_VER
  #include <io.h>
  #define isatty _isatty
#else
  #include <unistd.h>
#endif


namespace black::support::internal {

  std::optional<std::string> debug_msgs_tag;

  void debug(
    const char *filename, size_t line, const char *format, 
    fmt::format_args args
  ) {
    if(!debug_msgs_tag)
      return;

    fmt::text_style style = {};

    if(isatty(fileno(stderr)))
      style = fmt::fg(fmt::color::orange) | fmt::emphasis::bold;

    std::string message = fmt::vformat(format, args);

    fmt::print(
      stderr, "{}:{}: {}:{}: {}\n", 
      *debug_msgs_tag, styled("debug", style),
      relative(filename), line, message
    );
  }

}
