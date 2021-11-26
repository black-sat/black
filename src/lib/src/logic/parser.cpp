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
#include <black/logic/past_remover.hpp>

#include <fmt/format.h>

#include <string>
#include <sstream>

namespace black::internal
{
  namespace {

    inline bool does_need_parens(formula parent, formula arg) {
      bool parens = false;
      if(arg.is<binary>()) {
        parens = (!parent.is<conjunction>() && !parent.is<disjunction>())
              || (parent.formula_type() != arg.formula_type());
      }
      if(arg.is<atom>() && arg.to<atom>()->rel().known_type().has_value())
        parens = true;

      return parens;
    }

    inline bool does_need_parens(term /* parent */, term arg) {
      return arg.match(
        [](application a) {
          return a.func().known_type().has_value();
        },
        [](otherwise) {
          return false;
        }
      );
    }

    template<typename T>
    std::string parens_if_needed(T t, bool needs_parens) {
      return needs_parens ? "(" + to_string(t) + ")" : to_string(t);
    }
  }

  std::string to_string(term t)
  {
    using namespace std::literals;
    return t.match(
      [&](constant c) {
        return fmt::format("{}", c.value());
      },
      [&](variable x) {
        if(auto name = x.label<std::string>(); name.has_value())
          return *name;
        if(auto fname = x.label<std::pair<term_id,int>>(); fname.has_value()) {
          term t2 = t.sigma()->from_id(fname->first);
          return
            fmt::format("<{},{}>", to_string(t2), fname->second);
        }
        return fmt::format("<{:x}>", to_underlying(term{x}.unique_id()));
      },
      [&](application a) {
        if(auto t2 = a.func().known_type(); t2) {
          if(t2 == function::type::negation)
            return fmt::format("-{}", to_string(a.arguments()[0]));
          else {
            term lhs = a.arguments()[0];
            term rhs = a.arguments()[1];
            return fmt::format("{} {} {}", 
              parens_if_needed(lhs, does_need_parens(a, lhs)),
              a.func().name(),
              parens_if_needed(rhs, does_need_parens(a, rhs))
            );
          }
            
        }
        std::string result = 
          a.func().name() + "(" + to_string(a.arguments()[0]);
        for(size_t i = 1; i < a.arguments().size(); ++i) {
          result += ", " + to_string(a.arguments()[i]);
        }
        result += ")";

        return result;
      },
      [&](next n) {
        return fmt::format("next({})", to_string(n.argument()));
      }
    );
  }

  std::string to_string(formula f)
  {
    using namespace std::literals;
    return f.match(
      [&](proposition p) {
        if(auto name = p.label<std::string>(); name.has_value())
          return *name;
        if(auto fname = p.label<std::pair<formula,int>>(); fname.has_value())
          return
            fmt::format("<{},{}>", to_string(fname->first), fname->second); // LCOV_EXCL_LINE
        if(auto fname = p.label<past_label>(); fname.has_value())
          return fmt::format("<{}>", to_string(fname->formula)); // LCOV_EXCL_LINE
        else
          return fmt::format("<{:x}>", to_underlying(formula{p}.unique_id()));
      },
      [](atom a) {
        if(auto t = a.rel().known_type(); t)
          return fmt::format(
            "{} {} {}", 
            to_string(a.terms()[0]),
            a.rel().name(), 
            to_string(a.terms()[1])
          );
        
        std::string result = 
          a.rel().name() + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      },
      [](boolean b) {
        return b.value() ? "True" : "False";
      },
      [](negation n) {
        auto arg = n.operand();
        bool needs_parens = does_need_parens(n, arg);
        return fmt::format("!{}", parens_if_needed(arg, needs_parens));
      },
      [](unary u) {
        auto arg = u.operand();
        bool needs_parens = does_need_parens(u, arg);
        return fmt::format("{}{}{}",
                            to_string(u.formula_type()),
                            needs_parens ? "" : " ",
                            parens_if_needed(arg, needs_parens));
      },
      [](binary b) {
        auto lhs = b.left(), rhs = b.right();
        return
          fmt::format("{} {} {}",
                      parens_if_needed(lhs, does_need_parens(b, lhs)),
                      to_string(b.formula_type()),
                      parens_if_needed(rhs, does_need_parens(b, rhs)));
      }
    );
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

  // Easy entry-point for parsing formulas
  std::optional<formula>
  parse_formula(alphabet &sigma, std::istream &stream,
                parser::error_handler error)
  {
    parser p{sigma, stream, std::move(error)};

    return p.parse();
  }
}

// Implementation of parser class
namespace black::internal
{
  std::optional<token> parser::peek() {
    return _lex.peek();
  }

  std::optional<token> parser::peek(token::type t, std::string const&err) {
    auto tok = peek();
    if(!tok || tok->token_type() != t)
      return error("Expected " + err);

    return tok;
  }

  std::optional<token> parser::consume() {
    auto tok = peek();
    if(tok)
      _lex.get();
    return tok;
  }

  std::optional<token> parser::consume(token::type t, std::string const&err) {
    auto tok = peek(t, err);
    if(tok)
      _lex.get();
    return tok;
  }

  std::optional<token> 
  parser::consume_punctuation(token::punctuation p) {
    auto tok = peek();
    if(!tok || !tok->is<token::punctuation>() ||
        tok->data<token::punctuation>() != p) {
      return error("Expected '" + std::string{to_string(p)} + "'");
    }
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
      return error("Expected formula");

    return parse_binary_rhs(0, *lhs);
  }

  std::optional<formula> parser::parse_binary_rhs(int prec, formula lhs) {
    while(1) {
      if(!peek() || precedence(*peek()) < prec)
         return {lhs};

      token op = *consume();

      std::optional<formula> rhs = parse_primary();
      if(!rhs)
        return error("Expected right operand to binary operator");

      if(!peek() || precedence(op) < precedence(*peek())) {
        rhs = parse_binary_rhs(prec + 1, *rhs);
        if(!rhs)
          return error("Expected right operand to binary operator");
      }

      black_assert(op.is<binary::type>());
      lhs = binary(*op.data<binary::type>(), lhs, *rhs);
    }
  }

  std::optional<formula> parser::parse_boolean()
  {
    black_assert(peek() && peek()->token_type() == token::type::boolean);

    std::optional<token> tok = consume();

    black_assert(tok);
    black_assert(tok->token_type() == token::type::boolean);

    return _alphabet.boolean(*tok->data<bool>());
  }

  std::optional<formula> parser::parse_atom()
  {
    std::optional<term> lhs = parse_term();
    if(!lhs)
      return {};

    if(!peek() || !peek()->is<relation::type>())
      return error(
        "Expected binary relation, found " + std::string{to_string(*peek())}
      );

    relation::type r = *peek()->data<relation::type>();
    consume();

    std::optional<term> rhs = parse_term();
    if(!rhs)
      return {};

    return atom(relation{r}, {*lhs, *rhs});
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
      return {}; // error raised by parse();

    if(!consume_punctuation(token::punctuation::right_paren))
      return {}; // error raised by consume()

    return formula;
  }

  std::optional<formula> parser::parse_primary() {
    if(!peek())
      return {};

    if(peek()->token_type() == token::type::boolean)
      return parse_boolean();
    if(peek()->token_type() == token::type::constant ||
       peek()->data<function::type>() == function::type::subtraction ||
       peek()->token_type() == token::type::identifier)
      return parse_atom();
    if(peek()->is<unary::type>())
      return parse_unary();
    if(peek()->is<token::punctuation>() &&
       peek()->data<token::punctuation>() == token::punctuation::left_paren)
       return parse_parens();

    return error("Expected formula");
  }

  std::optional<term> parser::parse_term() {
    std::optional<term> lhs = parse_term_primary();
    if(!lhs)
      return error("Expected term");
    
    return parse_term_binary_rhs(0, *lhs);
  }

  std::optional<term> parser::parse_term_primary() {
    if(!peek())
      return {};

    if(peek()->token_type() == token::type::constant)
      return parse_term_constant();

    if(peek()->data<function::type>() == function::type::subtraction)
      return parse_term_unary_minus();

    if(peek()->token_type() == token::type::identifier)
      return parse_term_var_or_func();

    return error("Expected term, found " + std::string{to_string(*peek())});
  }

  std::optional<term> parser::parse_term_binary_rhs(int, term lhs) {
    return lhs;
  }

  std::optional<term> parser::parse_term_constant() {
    black_assert(peek());
    black_assert(peek()->token_type() == token::type::constant);

    token tok = *peek();
    consume();

    return _alphabet.constant(*tok.data<int>());
  }

  std::optional<term> parser::parse_term_unary_minus() {
    black_assert(peek());
    black_assert(peek()->data<function::type>() == function::type::subtraction);

    consume();
    std::optional<term> t = parse_term();
    if(!t)
      return {};
    return application(function{function::type::negation}, {*t});
  }

  std::optional<term> parser::parse_term_var_or_func() {
    black_assert(peek());
    black_assert(peek()->token_type() == token::type::identifier);

    std::string id{*peek()->data<std::string_view>()};
    consume();

    // if there is no open paren this is a simple variable
    if(!peek() || 
        peek()->data<token::punctuation>() != token::punctuation::left_paren)
      return _alphabet.var(id);

    // otherwise it is a function application
    std::vector<term> terms;
    do {      
      consume();
      std::optional<term> t = parse_term();
      if(!t)
        return {};
      terms.push_back(*t);
    } while(
      peek() && 
      peek()->data<token::punctuation>() == token::punctuation::comma
    );

    if(!consume_punctuation(token::punctuation::right_paren))
      return {};

    return application(function{id}, terms);
  }

} // namespace black::internal
