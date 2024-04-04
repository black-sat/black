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

namespace black::io {

  struct buffer {
    std::string _data;
    size_t _pos = -1; // yes I know
    std::vector<size_t> _lines;
    bool _newline = true;

    buffer(std::string data) : _data{std::move(data)} { get(); }

    size_t position() const { return _pos; }

    char peek() { 
      if(_pos >= _data.size())
        return EOF;
      return _data[_pos]; 
    }

    char get() {
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

    std::string_view range(size_t begin, size_t end) {
      black_assert(begin < _data.size());
      return std::string_view(_data).substr(begin, end - begin);
    }

  };

  struct lexer::impl_t 
  {
    std::filesystem::path _path;
    buffer _buffer;
    token _current;

    impl_t(std::filesystem::path path, std::string in) 
      : _path{path}, _buffer{std::move(in)} { }
    
    impl_t(std::string in) : _buffer{std::move(in)} { }

    token get();

  };

  lexer::lexer(std::filesystem::path path, std::string in)
    : _impl{std::make_unique<impl_t>(std::move(path), std::move(in))} { get();}
  
  lexer::lexer(std::string in)
    : _impl{std::make_unique<impl_t>(std::move(in))} { get(); }

  lexer::~lexer() = default;

  lexer::lexer(lexer &&) = default;
  lexer &lexer::operator=(lexer &&) = default;

  token lexer::peek() const { return _impl->_current; }

  token lexer::get() { return _impl->get(); }

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

  token lexer::impl_t::get() 
  {
    while(space(_buffer.peek()))
      _buffer.get();

    if(_buffer.peek() == EOF)
      return _current = token::eof{};

    if(alpha(_buffer.peek())) {
      size_t begin = _buffer.position();
      size_t end = begin;
      do {
        end++;
        _buffer.get();
      } while(alphanum(_buffer.peek()));

      return _current = token::identifier{_buffer.range(begin, end)};
    }

    if(punct(_buffer.peek())) {
      size_t begin = _buffer.position();
      size_t end = begin;
      do {
        end++;
        _buffer.get();
      } while(alphanum(_buffer.peek()));

      return _current = token::punctuation{_buffer.range(begin, end)};
    }

    if(num(_buffer.peek())) {
      bool integer = true;
      size_t begin = _buffer.position();
      size_t end = begin;

      do {
        end++;
        _buffer.get();
      } while(num(_buffer.peek()));
      
      // decimal dot and decimal part
      if(_buffer.peek() == '.') {
        integer = false;
        end++;
        _buffer.get();

        do {
          end++;
          _buffer.get();
        } while(num(_buffer.peek()));
      }

      // scientific notation
      if(_buffer.peek() == 'E' || _buffer.peek() == 'e') {
        integer = false;
        end++;
        _buffer.get();
        
        // exponent sign
        if(_buffer.peek() == '-' || _buffer.peek() == '+') {
          end++;
          _buffer.get();
        }

        // at least a digit is required for the exponent
        if(!num(_buffer.peek()))
          return _current = token::invalid{_buffer.range(begin, end)};

        do {
          end++;
          _buffer.get();
        } while(num(_buffer.peek()));
      }
        
      std::string_view range = _buffer.range(begin, end);
      const char *first = &range[0];
      const char *last = first + range.size();
      
      if(integer) {
        uint64_t value = 0;
        auto result = std::from_chars(first, last, value);

        if(result.ptr != last)
          return _current = token::invalid{range};
        
        return _current = token::integer{value};
      } else {
        char *p = nullptr;
        double value = std::strtod(first, &p);

        if(p != last)
          return _current = token::invalid{range};        
  
        return _current = token::real{value};
      }
    }

    return _current = token{};
  }
  
}