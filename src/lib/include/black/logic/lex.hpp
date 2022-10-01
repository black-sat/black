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

#ifndef BLACK_LEX_H_
#define BLACK_LEX_H_

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>

#include <cassert>
#include <cctype>

#include <optional>
#include <variant>
#include <vector>

namespace black_internal::lexer_details
{
  // Type representing tokens generated from the lexer.
  struct token
  {
    enum class type : uint8_t {
      invalid = 0,
      boolean,
      integer,
      real,
      identifier,
      quantifier,
      comparison,
      unary_term_operator,
      binary_term_operator,
      unary_operator,
      binary_operator,
      punctuation
    };

    // Type of non-logical tokens. Only parens, for now.
    enum class punctuation : uint8_t {
      // non-logical tokens
      left_paren,
      right_paren,
      comma,
      dot
    };

             token()              : _data{std::monostate{}} { }
    explicit token(bool b)        : _data{b} { }
    explicit token(int64_t c)     : _data{c} { }
    explicit token(double d)      : _data{d} { }
    explicit token(std::string s) : _data{std::move(s)} { }
    explicit token(logic::quantifier<logic::LTLPFO>::type k)  : _data{k} { }
    explicit token(logic::comparison<logic::LTLPFO>::type t)  : _data{t} { }
    explicit token(logic::unary_term<logic::LTLPFO>::type t)  : _data{t} { }
    explicit token(logic::binary_term<logic::LTLPFO>::type t) : _data{t} { }
    explicit token(logic::unary<logic::LTLPFO>::type t)       : _data{t} { }
    explicit token(logic::binary<logic::LTLPFO>::type t)      : _data{t} { }
    explicit token(punctuation s) : _data{s} { }

    template<typename T>
    bool is() const {
      return std::holds_alternative<T>(_data);
    }

    template<typename T>
    std::optional<T> data() const {
      if(auto p = std::get_if<T>(&_data); p)
        return {*p};
      return std::nullopt;
    }

    type token_type() const { return static_cast<type>(_data.index()); }

    friend std::string to_string(token const &tok);

  private:
    // data related to recognized tokens
    std::variant<
      std::monostate,            // invalid tokens
      bool,                      // booleans
      int64_t,                   // integers
      double,                    // reals
      std::string,               // identifiers
      logic::quantifier<logic::LTLPFO>::type,  // exists/forall
      logic::comparison<logic::LTLPFO>::type,  // =, !=, <logic::=, etc...
      logic::unary_term<logic::LTLPFO>::type,  // unary minus, next, wnext, ...
      logic::binary_term<logic::LTLPFO>::type, // +, -, *, /
      logic::unary<logic::LTLPFO>::type,       // unary operator
      logic::binary<logic::LTLPFO>::type,      // binary operator
      punctuation                // any non-logical token
    > _data;
  };

  std::string to_string(token::punctuation p);
  std::string to_string(token const &tok);

  class BLACK_EXPORT lexer
  {
  public:
    using error_handler = std::function<void(std::string)>;

    explicit lexer(std::istream &stream, error_handler error) 
      : _stream(stream), _error{error} {}  
    
    std::optional<token> get() { return _token = _lex(); }
    std::optional<token> peek() const { return _token; }

    static bool is_identifier_char(int c);
    static bool is_initial_identifier_char(int c);
    static bool is_keyword(std::string_view s);

  private:
    static std::pair<std::string_view, token> _keywords[31];
    std::optional<token> _lex();
    std::optional<token> _identifier();
    std::optional<token> _raw_identifier();

    std::optional<token> _token = std::nullopt;
    std::istream &_stream;
    error_handler _error;
  };

  std::ostream &operator<<(std::ostream &s, token const &t);
}

namespace black_internal {
  using lexer_details::lexer;
  using lexer_details::token;
}

#endif // LEX_H_
