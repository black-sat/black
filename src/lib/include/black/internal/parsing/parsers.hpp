//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_INTERNAL_PARSING_PARSERS_HPP
#define BLACK_INTERNAL_PARSING_PARSERS_HPP

namespace black::parsing {
  //!
  //! Matches a literal string and fails without consuming input otherwise.
  //! Output: none
  //!
  inline constexpr auto exactly(std::string_view str) {
    return wrap{
      [=](const char *begin, const char *end) -> result<std::monostate> {
        if(begin == end || size_t(end - begin) < str.size())
          return failure{begin};

        if(std::string_view{begin, str.size()} == str)
          return success{begin + str.size()};
        
        return failure{begin};
      }
    };
  }

  //!
  //! Matches the longest non-empty sequence of characters satisfying `pred`.
  //! Output: an `std::string_view` of the matched sequence.
  //!
  inline constexpr auto pattern(auto pred) {
    return wrap{
      [=](const char *begin, const char *end) -> result<std::string_view> {
        
        auto it = begin;
        while(it != end && pred(*it))
          it++;

        if(it == begin)
          return failure{begin};

        return success{it, std::string_view{begin, it}};
      }
    };
  }
}

#endif // BLACK_INTERNAL_PARSING_PARSERS_HPP
