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

#include <black/logic/alphabet.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/lex.hpp>

#include <fmt/format.h>

#include <string>
#include <sstream>

namespace black::details
{
  namespace {
    std::string to_string_impl(formula f, std::optional<int>)
    {
      using namespace std::literals;
      return f.match(
        [&](atom a) {
          if(auto name = a.label<std::string>(); name.has_value())
            return *name;
          if(auto fname = a.label<std::pair<formula,int>>(); fname.has_value())
            return
              fmt::format("<{},{}>", to_string(fname->first), fname->second);
          else
            return "<?>"s;
        },
        [](boolean b) {
          return b.value() ? "true"s : "false"s;
        },
        [](unary u) {
          auto arg = u.operand();
          auto sarg = arg.is<binary>() ? fmt::format("({})", to_string(arg))
                                       : fmt::format("{}", to_string(arg));
          return fmt::format("{}{}", to_string(u.formula_type()), sarg);
        },
        [](binary b) {
          auto lhs = b.left(), rhs = b.right();
          auto slhs = lhs.is<binary>() ? fmt::format("({})", to_string(lhs))
                                       : fmt::format("{}", to_string(lhs));
          auto srhs = rhs.is<binary>() ? fmt::format("({})", to_string(rhs))
                                       : fmt::format("{}", to_string(rhs));
          return
            fmt::format("{} {} {}", slhs, to_string(b.formula_type()), srhs);
        }
      );
    }
  }

  std::string to_string(formula f) {
    return to_string_impl(f, {});
  }

  // Easy entry-point for parsing formulas
  std::optional<formula>
  parse_formula(alphabet &sigma, std::string const&s,
                parser::error_handler error)
  {
    std::stringstream stream{s, std::stringstream::in};
    parser p{sigma, stream, std::move(error)};

    return p.parse();
  }
}

// Implementation of parser class
namespace black::details
{
  optional<token> parser::peek() {
    return _lex.peek();
  }

  optional<token> parser::peek(token::type t, std::string const&err) {
    auto tok = peek();
    if(!tok || tok->token_type() != t)
      return error("Expected " + err);

    return tok;
  }

  optional<token> parser::consume() {
    auto tok = peek();
    if(tok)
      _lex.get();
    return tok;
  }

  optional<token> parser::consume(token::type t, std::string const&err) {
    auto tok = peek(t, err);
    if(tok)
      _lex.get();
    return tok;
  }

  std::nullopt_t parser::error(std::string const&s) {
    _error(s);
    return std::nullopt;
  }

  std::optional<formula> parser::parse() {
    std::optional<formula> lhs = parse_primary();
    if(!lhs)
      return {};

    return parse_binary_rhs(0, *lhs);
  }

  std::optional<formula> parser::parse_binary_rhs(int prec, formula lhs) {
    while(1) {
      if(!peek() || precedence(*peek()) < prec)
         return {lhs};

      token op = *consume();

      std::optional<formula> rhs = parse_primary();
      if(!rhs)
        return {};

      if(!peek() || precedence(op) < precedence(*peek())) {
        rhs = parse_binary_rhs(prec + 1, *rhs);
        if(!rhs)
          return {};
      }

      black_assert(op.is<binary::type>());
      lhs = binary(*op.data<binary::type>(), lhs, *rhs);
    }
  }

  std::optional<formula> parser::parse_atom()
  {
    // Assume we are on an atom
    black_assert(peek() && peek()->token_type() == token::type::atom);

    std::optional<token> tok = consume();

    black_assert(tok);
    black_assert(tok->token_type() == token::type::atom);

    return _alphabet.var(*tok->data<std::string_view>());
  }

  std::optional<formula> parser::parse_unary()
  {
    std::optional<token> op = consume(); // consume unary op
    black_assert(op && op->is<unary::type>());

    std::optional<formula> formula = parse_primary();
    if(!formula)
      return {};

    return unary(*op->data<unary::type>(), *formula);
  }

  std::optional<formula> parser::parse_parens() {
    black_assert(peek());
    black_assert(peek()->is<token::punctuation>());
    black_assert(
      peek()->data<token::punctuation>() == token::punctuation::left_paren);

    consume(); // Consume left paren '('

    std::optional<formula> formula = parse();
    if(!formula)
      return {};

    if(!consume(token::type::punctuation, "')'"))
      return {};

    return formula;
  }

  std::optional<formula> parser::parse_primary() {
    if(!peek())
      return {};

    if(peek()->token_type() == token::type::atom)
      return parse_atom();
    if(peek()->is<unary::type>())
      return parse_unary();
    if(peek()->is<token::punctuation>() &&
       peek()->data<token::punctuation>() == token::punctuation::left_paren)
       return parse_parens();

    return error("Expected formula");
  }

} // namespace black::details