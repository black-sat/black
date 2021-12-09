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
#include <black/logic/formula.hpp>

#include <cassert>
#include <cctype>

#include <optional>
#include <variant>
#include <vector>

namespace black::internal
{
  // Type representing tokens generated from the lexer.
  struct token
  {
    enum class type : uint8_t {
      boolean = 0,
      integer,
      real,
      identifier,
      keyword,
      relation,
      function,
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

    // we only have one keyword yet
    enum class keyword : uint8_t {
      next,
      wnext,
      exists,
      forall
    };

    token(bool b)               : _data{b} { }
    token(int c)                : _data{c} { }
    token(double d)             : _data{d} { }
    token(std::string s)        : _data{std::move(s)} { }
    token(keyword k)            : _data{k} { }
    token(relation::type t)     : _data{t} { }
    token(function::type t)     : _data{t} { }
    token(unary::type t)        : _data{t} { }
    token(binary::type t)       : _data{t} { }
    token(punctuation s) : _data{s} { }

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
      bool,           // booleans
      int,            // integers
      double,         // reals
      std::string,    // identifiers
      keyword,        // keywords
      relation::type, // known relations
      function::type, // known functions
      unary::type,    // unary operator
      binary::type,   // binary operator
      punctuation     // any non-logical token
    > _data;
  };

  inline std::string to_string(unary::type t)
  {
    constexpr std::string_view toks[] = {
      "!",  // negation
      "X",  // tomorrow
      "wX", // weak tomorrow
      "Y",  // yesterday
      "Z",  // weak yesterday
      "G",  // always
      "F",  // eventually
      "O",  // once
      "H",  // historically
    };

    return std::string{
      toks[to_underlying(t) - to_underlying(unary::type::negation)]
    };
  }

  inline std::string to_string(binary::type t) {
    constexpr std::string_view toks[] = {
      "&",   // conjunction
      "|",   // disjunction
      "->",  // implication
      "<->", // iff
      "U",   // until
      "R",   // release
      "W",   // weak until
      "M",   // strong release
      "S",   // since
      "T",   // triggered
    };

    return std::string{
      toks[to_underlying(t) - to_underlying(binary::type::conjunction)]
    };
  }

  inline std::string to_string(token::keyword k) {
    constexpr std::string_view toks[] = {
      "next",
      "wnext",
      "exists",
      "forall"
    };

    return std::string{toks[to_underlying(k)]};
  }

  inline std::string to_string(relation::type t) {
    constexpr std::string_view toks[] = {
      "=",  // equal
      "!=", // not_equal
      "<",  // less_than
      "<=", // less_than_equal
      ">",  // greater_than
      ">="  // greater_than_equal
    };

    return std::string{toks[to_underlying(t)]};
  }

  inline std::string to_string(function::type t) {
    constexpr std::string_view toks[] = {
      "-",   // negation
      "-",   // subtraction
      "+",   // addition
      "*",   // multiplication
      "/",   // division
      "mod", // modulo
    };

    return std::string{toks[to_underlying(t)]};
  }

  inline std::string to_string(token::punctuation p) {
    constexpr std::string_view toks[] = {
      "(", // left_paren
      ")", // right_paren
      ",", // comma
      "."  // dot
    };

    return std::string{toks[to_underlying(p)]};
  }

  inline std::string to_string(token const &tok)
  {
    using namespace std::literals;

    return std::visit( overloaded {
      [](bool b)               { return b ? "true"s : "false"s; },
      [](int c)                { return to_string(c); },
      [](double d)             { return to_string(d); },
      [](std::string s)        { return s; },
      [](token::keyword k)     { return to_string(k); },
      [](relation::type t)     { return to_string(t); },
      [](function::type t)     { return to_string(t); },
      [](unary::type t)        { return to_string(t); },
      [](binary::type t)       { return to_string(t); },
      [](token::punctuation p) { return to_string(p); }
    }, tok._data);
  }

  class BLACK_EXPORT lexer
  {
  public:
    explicit lexer(std::istream &stream) : _stream(stream) {}
    std::optional<token> get() { return _token = _lex(); }
    std::optional<token> peek() const { return _token; }

  private:
    std::optional<token> _lex();
    std::optional<token> _identifier();
    bool _is_identifier_char(int c);
    bool _is_initial_identifier_char(int c);

    std::optional<token> _token = std::nullopt;
    std::istream &_stream;
  };

  std::ostream &operator<<(std::ostream &s, token const &t);
}

#endif // LEX_H_
