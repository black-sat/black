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

#include <iostream>
#include <cassert>
#include <cctype>

#include <optional>
#include <variant>
#include <vector>

namespace black::internal
{
  // Type representing tokens generated from the lexer.
  // WARNING: tokens must not outlive their originating lexer object.
  struct token
  {
    enum class type : uint8_t {
      boolean = 0,
      proposition,
      unary_operator,
      binary_operator,
      punctuation
    };

    // Type of non-logical tokens. Only parens, for now.
    enum class punctuation : uint8_t {
      // non-logical tokens
      left_paren,
      right_paren
    };

    constexpr token(bool b)               : _data{b} { }
    constexpr token(std::string_view s)   : _data{s} { }
    constexpr token(unary::type t)        : _data{t} { }
    constexpr token(binary::type t)       : _data{t} { }
    constexpr token(punctuation s) : _data{s} { }

    template<typename T>
    constexpr bool is() const {
      return std::holds_alternative<T>(_data);
    }

    template<typename T>
    constexpr std::optional<T> data() const {
      if(auto p = std::get_if<T>(&_data); p)
        return {*p};
      return std::nullopt;
    }

    type token_type() const { return static_cast<type>(_data.index()); }

    friend constexpr std::string_view to_string(token const &tok);

  private:
    // data related to recognized tokens
    std::variant<
      bool,             // booleans
      std::string_view, // propositions
      unary::type,      // unary operator
      binary::type,     // binary operator
      punctuation       // any non-logical token
    > _data;
  };

  constexpr std::string_view to_string(unary::type const& t)
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

    return toks[to_underlying(t) - to_underlying(unary::type::negation)];
  }

  constexpr std::string_view to_string(binary::type const& t) {
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

    return toks[to_underlying(t) - to_underlying(binary::type::conjunction)];
  }

  constexpr std::string_view to_string(token const &tok)
  {
    using namespace std::literals;

    return std::visit( overloaded {
      [](bool b)             { return b ? "true"sv : "false"sv; },
      [](std::string_view s) { return s; },
      [](unary::type t)      { return to_string(t); },
      [](binary::type t)     { return to_string(t); },
      [](token::punctuation s) {
        switch(s) {
          case token::punctuation::left_paren:
            return "("sv;
          case token::punctuation::right_paren:
            return ")"sv;
        }
      }
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

    std::vector<std::string> _lexed_identifiers;
  };

  std::ostream &operator<<(std::ostream &s, token const &t);
}

#endif // LEX_H_
