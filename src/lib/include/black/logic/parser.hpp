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


#ifndef PARSER_H_
#define PARSER_H_

#include <black/logic/alphabet.hpp>
#include <black/logic/lex.hpp>

#include <istream>
#include <ostream>
#include <functional>

namespace black::internal
{
  //
  // Class to parse LTL formulas
  //
  class parser
  {
  public:
    using error_handler = std::function<void(std::string)>;

    parser(alphabet &sigma, std::istream &stream, error_handler error)
      : _alphabet(sigma), _lex(stream), _error(std::move(error))
    {
      _lex.get();
    }

    std::optional<formula> parse();

  private:
    std::optional<token> peek();
    std::optional<token> consume();
    std::optional<token> peek(token::type, std::string const&err);
    std::optional<token> consume(token::type, std::string const&err);
    std::nullopt_t error(std::string const&s);

    std::optional<formula> parse_binary_rhs(int precedence, formula lhs);
    std::optional<formula> parse_boolean();
    std::optional<formula> parse_atom();
    std::optional<formula> parse_unary();
    std::optional<formula> parse_parens();
    std::optional<formula> parse_primary();

  private:
    alphabet &_alphabet;
    lexer _lex;
    std::function<void(std::string)> _error;
  };

  // Easy entry-point for parsing formulas
  std::optional<formula>
  parse_formula(alphabet &sigma, std::string const&s,
                parser::error_handler error);

  inline std::optional<formula>
  parse_formula(alphabet &sigma, std::string const&s) {
    return parse_formula(sigma, s, [](auto){});
  }

  std::string to_string(formula f);

  inline std::ostream &operator<<(std::ostream &stream, formula const&f) {
    return (stream << to_string(f));
  }

  constexpr std::optional<int> precedence(token const&tok)
  {
    // Attention: this must remain in sync with token::token_type
    constexpr std::optional<int> ops[] = {
      {30}, // conjunction
      {20}, // disjunction
      {40}, // then
      {40}, // iff
      {50}, // until
      {50}, // release
      {50}, // since
      {50}, // triggered
    };

    if(auto t = tok.data<binary::type>(); t)
      return ops[to_underlying(*t) - to_underlying(binary::type::conjunction)];

    return std::nullopt;
  }

} // namespace black::internal

// Exported names
namespace black {
  using internal::parser;
  using internal::parse_formula;
  using internal::to_string;
}

#endif // PARSER_H_
