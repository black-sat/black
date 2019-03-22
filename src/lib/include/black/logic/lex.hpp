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

    constexpr token(token_type t) : type(t) {
      black_assert(t != token::boolean);
      black_assert(t != token::atom);
    }
    constexpr token(bool b) : type(token::boolean), data(b) {}
    token(std::string a) : type(token::atom), data(std::move(a)) {}

    bool is_logical_symbol() const {
      return to_underlying(type) <= to_underlying(triggered);
    }

    bool is_unary_op() const {
      return to_underlying(type) >= to_underlying(negation) &&
             to_underlying(type) <= to_underlying(historically);
    }

    bool is_binary_op() const {
      return to_underlying(type) >= to_underlying(conjunction) &&
             to_underlying(type) <= to_underlying(triggered);
    }

    constexpr std::optional<formula::type> formula_type() const;

    constexpr std::optional<unary::operator_type> unary_type() const;

    constexpr std::optional<binary::operator_type> binary_type() const;

    token_type type;
    std::variant<std::monostate, bool, std::string> data;
  };

  constexpr std::string_view to_string(unary::operator_type const& t) {
    constexpr std::string_view toks[] = {
      "!", // negation
      "X", // tomorrow
      "Y", // yesterday
      "G", // always
      "F", // eventually
      "P", // past
      "H", // historically
    };

    black_assert(to_underlying(t) < sizeof(toks));

    return toks[to_underlying(t)];
  }

  constexpr std::string_view to_string(binary::operator_type const& t) {
    constexpr std::string_view toks[] = {
      "&&",  // conjunction
      "||",  // disjunction
      "->",  // then
      "<->", // iff
      "U",   // until
      "R",   // release
      "S",   // since
      "T",   // triggered
    };

    black_assert(to_underlying(t) < sizeof(toks));

    return toks[to_underlying(t)];
  }

  constexpr std::string_view to_string(token const &tok)
  {
    switch(tok.type) {
      case token::boolean:
        black_assert(std::holds_alternative<bool>(tok.data));
        return std::get<bool>(tok.data) ? "true" : "false";
      case token::atom:
        black_assert(std::holds_alternative<std::string>(tok.data));
        return std::get<std::string>(tok.data);
      case token::left_paren:
        return "(";
      case token::right_paren:
        return ")";
      default:
        if(tok.is_unary_op())
          return to_string(*tok.unary_type());
        else if(tok.is_binary_op())
          return to_string(*tok.binary_type());
    }
    black_unreachable();
  }

  constexpr std::optional<formula::type> token::formula_type() const {
    if(is_logical_symbol())
      return {formula::type{to_underlying(type)}};
    return {};
  }

  constexpr std::optional<unary::operator_type> token::unary_type() const {
    uint8_t u = to_underlying(type);
    if(is_unary_op()) {
      uint8_t v = u - uint8_t(token_type::negation);
      return {from_underlying<unary::operator_type>(v)};
    }

    return {};
  }

  constexpr std::optional<binary::operator_type> token::binary_type() const {
    uint8_t u = to_underlying(type);
    if(is_binary_op()) {
      uint8_t v = u - uint8_t(token_type::conjunction);
      return {from_underlying<binary::operator_type>(v)};
    }
    return {};
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
