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

#include <format>

namespace black::support::internal {
  
  extern std::optional<std::string> debug_msgs_tag;

  inline void enable_debug_msgs(std::string const &t) { debug_msgs_tag = t; }
  inline void disable_debug_msgs() { debug_msgs_tag = {}; }
  inline bool are_debug_msgs_enabled() { return debug_msgs_tag.has_value(); }

  void debug(
    const char *filename, size_t line, const char *format, 
    std::format_args args
  );

}

#define black_debug(Message, ...)                                   \
  black::support::internal::debug (                                 \
    __FILE__, __LINE__, Message, std::make_format_args(__VA_ARGS__) \
  )

namespace black::support {
  using internal::enable_debug_msgs;
  using internal::disable_debug_msgs;
  using internal::are_debug_msgs_enabled;
}

#endif // BLACK_SUPPORT_DEBUG_HPP
