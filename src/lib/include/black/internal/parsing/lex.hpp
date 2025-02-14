//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2025 Nicola Gigante
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

#ifndef BLACK_PARSING_LEX_HPP
#define BLACK_PARSING_LEX_HPP

namespace black::parsing {

  using namespace std::literals;

  inline parser<size_t> integer() {
    return [] -> parsed<size_t> {
      size_t x = 0;
      int p = 1;
      
      for(char c : co_await reversed(some(chr(&isdigit)))) {
        x += (c - '0') * p;
        p *= 10;
      }
      
      co_return x;
    };
  }

  parser<std::string> string(predicate_for<char> auto pred) {
    return collect<std::basic_string>(many(chr(pred)));
  }

  inline parser<std::string> string(std::string_view str) {
    return collect<std::basic_string>(
      sequence(transform(yield(str), [](char c) { return chr(c); }))
    );
  }

  template<typename T>
  parser<T> token(parser<T> p) {
    return many(chr(&isspace)) + p;
  }

  inline bool is_id_start(char c) {
    return isalpha(c) || c == '_';
  }

  inline bool is_id_char(char c) {
    return is_id_start(c) || isdigit(c);
  }

  inline parser<std::string> identifier(std::string_view str) {
    return require(string(&is_id_char), [=](auto&& id) { return id == str; });
  }

}

#endif // BLACK_PARSING_LEX_HPP

