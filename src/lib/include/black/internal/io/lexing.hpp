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

#ifndef BLACK_INTERNAL_IO_LEXER_HPP
#define BLACK_INTERNAL_IO_LEXER_HPP

#include <ostream>
#include <string>
#include <filesystem>
#include <format>


namespace black::io {

  class buffer 
  {
  public:
    explicit buffer(std::string data);
    
    buffer(buffer const&) = delete;
    buffer(buffer &&) = default;
    
    buffer &operator=(buffer const&) = delete;
    buffer &operator=(buffer &&) = default;

    size_t position() const;

    char peek() const;

    char get();

    std::string_view range(size_t begin, size_t end) const;

  private:
    std::string _data;
    size_t _pos = -1; // yes I know
    std::vector<size_t> _lines;
    bool _newline = true;
  };

  using lex_acceptor_t = bool (*)(std::string_view str);

  token lex(buffer *buf, lex_acceptor_t symbols, lex_acceptor_t keywords);

  template<parselet P>
  token lex(buffer *buf, P) {
    return lex(buf, &P::is_symbol, &P::is_keyword);
  }

}



#endif // BLACK_INTERNAL_IO_LEXER_HPP
