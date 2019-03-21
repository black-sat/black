/*
 Copyright (c) 2014, Nicola Gigante
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * The names of its contributors may not be used to endorse or promote
 products derived from this software without specific prior written
 permission.
 */

#ifndef BLACK_LEX_H_
#define BLACK_LEX_H_

#include <black/logic/formula.hpp>

#include <iostream>
#include <cassert>
#include <cctype>

#include <optional>
#include <variant>

namespace black::details
{
  #define declare_token(Name) Name = formula::type,

  struct token {

    // Type of recognised tokens.
    // WARNING: Keep the order in synch with formula::type
    enum token_type : uint8_t {
      // atomic formulas
      boolean,
      atom,
      negation,

      // unary operators
      tomorrow,
      yesterday,
      always,
      eventually,
      past,
      historically,

      // binary operators
      conjunction,
      disjunction,
      then,
      iff,
      until,
      release,
      since,
      triggered,

      // non-logical tokens
      left_paren,
      right_paren
    };

    token(token_type t) : type(t) {
      black_assert(t != token::boolean);
      black_assert(t != token::atom);
    }
    token(bool b) : type(token::boolean), data(b) {}
    token(std::string a) : type(token::atom), data(std::move(a)) {}

    token_type type;
    std::variant<std::monostate, bool, std::string> data;
  };

  constexpr int binary_precedence(binary::operator_type bintype) {
    // Attention: this must remain in sync with binary::operator_type
    constexpr int binops[] = {
      30, // conjunction
      20, // disjunction
      40, // then
      40, // iff
      50, // until
      50, // release
      50, // since
      50  // triggered
    };

    return binops[to_underlying(bintype)];
  }

  constexpr std::string_view to_string(token const &tok)
  {
    constexpr std::string_view toks[] = {
      // atoms and boolean printed separately
      "!", // negation,
      "X", // tomorrow,
      "Y", // yesterday,
      "H", // always,
      "F", // eventually,
      "P", // past,
      "H", // historically,
      "&&", // conjunction,
      "||", // disjunction,
      "->", // then,
      "<->", // iff,
      "U", // until,
      "R", // release,
      "S", // since,
      "T", // triggered,
      "(", // left_paren,
      ")"  // right_paren
    };

    switch(tok.type) {
      case token::boolean:
        std::cout << "tok.data index: " << tok.data.index() << "\n";
        black_assert(std::holds_alternative<bool>(tok.data));
        return std::get<bool>(tok.data) ? "true" : "false";
      case token::atom:
        black_assert(std::holds_alternative<std::string>(tok.data));
        return std::get<std::string>(tok.data);
      default:
        return toks[to_underlying(tok.type) - 2];
    }
  }

  class lexer
  {
  public:
    explicit lexer(std::istream &stream) : _stream(stream) {}
    std::optional<token> get() { return _token = _lex(); }
    std::optional<token> peek() const { return _token; }

  private:
    std::optional<token> _lex();

    std::optional<token> _token = std::nullopt;
    std::istream &_stream;
  };

  std::ostream &operator<<(std::ostream &s, token const &t);
}

#endif // LEX_H_
