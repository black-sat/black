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

  class token 
  {
  public:
    struct null        { std::monostate   value; };
    struct eof         { std::monostate   value; };
    struct invalid     { std::string_view value; };
    struct identifier  { std::string_view value; };
    struct integer     { uint64_t         value; };
    struct real        { double           value; };
    struct punctuation { std::string_view value; };

    using token_t = 
      std::variant<null, eof, invalid, identifier, integer, real, punctuation>;

    template<typename T>
      requires support::is_in_variant_v<T, token::token_t>
    friend bool operator==(T t1, T t2) { return t1.value == t2.value; }

    token()               : _data{null{}} { }
    token(null)           : _data{null{}} { }
    token(eof)            : _data{eof{}} { }
    token(invalid inv)    : _data{inv} { }
    token(identifier id)  : _data{id} { }
    token(integer value)  : _data{value} { }
    token(real value)     : _data{real{value}} { }
    token(punctuation p)  : _data{p} { }
 
    token(token const&) = default;
    token(token &&) = default;
    
    token &operator=(token const&) = default;
    token &operator=(token &&) = default;

    bool operator==(token const&) const = default;

    template<typename T>
      requires support::is_in_variant_v<T, token_t>
    bool is() const {
      return std::holds_alternative<T>(_data);
    }

    template<typename T>
      requires support::is_in_variant_v<T, token_t>
    std::optional<T> get() const {
      if(is<T>())
        return std::get<T>(_data);
      return {};
    }

    explicit operator bool() const { 
      return !is<null>() && !is<invalid>() && !is<eof>();
    }

  private:
    token_t _data;
  };

  class lexer 
  {
  public:
    lexer(std::filesystem::path path, std::string in);
    explicit lexer(std::string in);
    ~lexer();

    lexer(lexer const&) = delete;
    lexer(lexer &&);
    
    lexer &operator=(lexer const&) = delete;
    lexer &operator=(lexer &&);

    token peek() const;
    token get();

    template<typename T>
      requires support::is_in_variant_v<T, token::token_t>
    token get() {
      token tok = get();
      if(tok.is<T>())
        return tok;
      return token::null{};
    }
    
    template<typename T>
      requires support::is_in_variant_v<T, token::token_t>
    token peek() {
      token tok = peek();
      if(tok.is<T>())
        return tok;
      return token::null{};
    }

  private:
    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

}

namespace black::support {

  template<>
  struct match_cases<black::io::token> 
    : support::variant_alternatives<black::io::token::token_t> { };

  template<typename T>
    requires support::is_in_variant_v<T, black::io::token::token_t>
  struct match_downcast<black::io::token, T> {
    static std::optional<T> downcast(black::io::token tok) {
      return tok.get<T>();
    }
  };
}

namespace black::io {
  inline std::ostream &operator<<(std::ostream &out, token const& tok) {
    return out << support::match(tok)(
      [&](token::null) { 
        return "token::null{}"; 
      },
      [&](token::eof) { 
        return "token::eof{}"; 
      },
      [&](token::invalid t) { 
        return std::format("token::invalid{{{:?}}}", t.value);
      },
      [&](token::identifier t) { 
        return std::format("token::identifier{{{:?}}}", t.value);
      },
      [&](token::integer t) { 
        return std::format("token::integer{{{}}}", t.value);
      },
      [&](token::real t) { 
        return std::format("token::real{{{}}}", t.value);
      },
      [&](token::punctuation t) { 
        return std::format("token::punctuation{{{:?}}}", t.value);
      }
    );
  }
}

#endif // BLACK_INTERNAL_IO_LEXER_HPP
