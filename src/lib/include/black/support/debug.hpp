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

#ifndef BLACK_SUPPORT_DEBUG_HPP
#define BLACK_SUPPORT_DEBUG_HPP

#include <fmt/format.h>

namespace black::support::internal {
  
  extern std::optional<std::string> tag;

  inline void enable_reporting(std::string t) { tag = t; }
  inline void disable_reporting() { tag = {}; }
  inline bool is_reporting_enabled() { return tag.has_value(); }

  void report(
    const char *filename, size_t line, const char *format, 
    fmt::format_args args
  );

}

#define black_report(Message, ...)                                      \
  do {                                                                  \
    if(black::support::internal::tag)                                   \
      black::support::internal::report(                                 \
        __FILE__, __LINE__, Message, fmt::make_format_args(__VA_ARGS__) \
      );                                                                \
  } while(false)

namespace black::support {
  using internal::enable_reporting;
  using internal::disable_reporting;
  using internal::is_reporting_enabled;
}

#endif // BLACK_SUPPORT_DEBUG_HPP
