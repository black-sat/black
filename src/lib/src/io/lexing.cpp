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

#include <black/io>

#include <cstring>
#include <charconv>
#include <iostream>

namespace black::io 
{

  buffer::buffer(std::string data) : _data{std::move(data)} { get(); }

  size_t buffer::position() const { return _pos; }

  char buffer::peek() const { 
    if(_pos >= _data.size())
      return EOF;
    return _data[_pos]; 
  }

  char buffer::get() {
    _pos++;

    if(_pos >= _data.size())
      return EOF;
    
    char c = _data[_pos];
    
    if(_newline) {
      _lines.push_back(_pos);
      _newline = false;
    }
      
    if(c == '\n')
      _newline = true;
    
    return c;
  }

  std::string_view buffer::range(size_t begin, size_t end) const {
    black_assert(begin < _data.size());
    return std::string_view(_data).substr(begin, end - begin);
  }

  static bool space(char c) { 
    return strchr(" \t\r\n", c) != nullptr;
  }
  
  static bool punct(char c) { 
    return strchr(R"--(,./<>?~!@#$%^&*()_-+=:;'"[]{})--", c) != nullptr;
  }

  static bool num(char c) {
    return c >= '0' && c <= '9';
  }

  static bool alphanum(char c) { 
    return strchr("abcdefghijklmnopqrstuvwxyz0123456789_", c) != nullptr;
  }
  
  static bool alpha(char c) { 
    return strchr("abcdefghijklmnopqrstuvwxyz_", c) != nullptr;
  }

  token lex(buffer *buf, lex_acceptor_t symbols, lex_acceptor_t keywords) 
  {
    while(space(buf->peek()))
      buf->get();

    if(buf->peek() == EOF)
      return token::eof{};

    if(alpha(buf->peek())) {
      size_t begin = buf->position();
      size_t end = begin;
      do {
        end++;
        buf->get();
      } while(alphanum(buf->peek()));

      auto data = buf->range(begin, end);
      if(keywords(data))
        return token::keyword{data};
      return token::identifier{data};
    }

    if(punct(buf->peek())) {
      size_t begin = buf->position();
      size_t end = begin;
      do {
        end++;
        buf->get();
      } while(punct(buf->peek()));

      // we return the longest that match
      for(size_t last = end; last > begin; last--)
        if(symbols(buf->range(begin, last)))
          return token::punctuation{buf->range(begin, last)};
      
      return token::invalid{buf->range(begin, end)};
    }

    if(num(buf->peek())) {
      bool integer = true;
      size_t begin = buf->position();
      size_t end = begin;

      do {
        end++;
        buf->get();
      } while(num(buf->peek()));
      
      // decimal dot and decimal part
      if(buf->peek() == '.') {
        integer = false;
        end++;
        buf->get();

        do {
          end++;
          buf->get();
        } while(num(buf->peek()));
      }

      // scientific notation
      if(buf->peek() == 'E' || buf->peek() == 'e') {
        integer = false;
        end++;
        buf->get();
        
        // exponent sign
        if(buf->peek() == '-' || buf->peek() == '+') {
          end++;
          buf->get();
        }

        // at least a digit is required for the exponent
        if(!num(buf->peek()))
          return token::invalid{buf->range(begin, end)};

        do {
          end++;
          buf->get();
        } while(num(buf->peek()));
      }
        
      std::string_view range = buf->range(begin, end);
      const char *first = &range[0];
      const char *last = first + range.size();
      
      if(integer) {
        uint64_t value = 0;
        auto result = std::from_chars(first, last, value);

        if(result.ptr != last)
          return token::invalid{range};
        
        return token::integer{value};
      } else {
        char *p = nullptr;
        double value = std::strtod(first, &p);

        if(p != last)
          return token::invalid{range};        
  
        return token::real{value};
      }
    }

    return token{};
  }
  
}