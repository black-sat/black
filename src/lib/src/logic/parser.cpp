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

#include <tsl/hopscotch_map.h>

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
        if(std::holds_alternative<int>(c.value()))
          return fmt::format("{}", std::get<int>(c.value()));
        else
          return fmt::format("{}", std::get<double>(c.value()));
      },
      [&](variable x) {
        if(auto name = x.label<std::string>(); name.has_value())
          return *name;
        if(auto fname = x.label<std::pair<term_id,size_t>>(); fname.has_value()) {
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
      },
      [&](wnext n) {
        return fmt::format("wnext({})", to_string(n.argument()));
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
        if(auto fname = p.label<std::pair<formula,size_t>>(); fname.has_value())
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
      [](quantifier q) {
        std::string qs = q.quantifier_type() == quantifier::type::exists ?
          "exists" : "forall";

        bool parens = 
          !q.matrix().is<boolean>() && 
          !q.matrix().is<proposition>() && 
          !q.matrix().is<atom>() && 
          !q.matrix().is<quantifier>();

        return fmt::format("{} {}.{}",
          qs, to_string(q.var()), 
          parens_if_needed(q.matrix(), parens)
        );
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
  std::optional<parser::result>
  parse_formula(alphabet &sigma, std::string const&s,
                parser::error_handler error)
  {
    std::stringstream stream{s, std::stringstream::in};
    parser p{sigma, stream, std::move(error)};

    return p.parse();
  }

  // Easy entry-point for parsing formulas
  std::optional<parser::result>
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
  struct parser::_parser_t {
    alphabet &_alphabet;
    lexer _lex;
    uint8_t _features = 0;
    std::function<void(std::string)> _error;
    tsl::hopscotch_map<std::string, int> _func_arities;
    tsl::hopscotch_map<std::string, int> _rel_arities;

    _parser_t(alphabet &sigma, std::istream &stream, error_handler error)
      : _alphabet(sigma), _lex(stream), _features{0}, _error(std::move(error))
    {
      _lex.get();
    }

    std::optional<token> peek();
    std::optional<token> consume();
    std::optional<token> peek(token::type, std::string const&err);
    std::optional<token> consume(token::type, std::string const&err);
    std::optional<token> consume_punctuation(token::punctuation p);
    std::nullopt_t error(std::string const&s);

    void set_features(token const &tok);

    std::optional<formula> parse_formula(bool quantified);
    std::optional<formula> parse_binary_rhs(int, formula, bool);
    std::optional<formula> parse_boolean();
    std::optional<formula> parse_atom();
    std::optional<formula> parse_quantifier();
    std::optional<formula> parse_unary(bool quantified);
    std::optional<formula> parse_parens(bool quantified);
    std::optional<formula> parse_primary(bool quantified);

    std::optional<formula> correct_term_to_formula(term t);

    std::optional<term> register_term(term t);
    std::optional<term> parse_term();
    std::optional<term> parse_term_primary();
    std::optional<term> parse_term_binary_rhs(int precedence, term lhs);
    std::optional<term> parse_term_constant();
    std::optional<term> parse_term_unary_minus();
    std::optional<term> parse_term_next();
    std::optional<term> parse_term_wnext();
    std::optional<term> parse_term_var_or_func();
    std::optional<term> parse_term_parens();
  };

  parser::parser(alphabet &sigma, std::istream &stream, error_handler error)
    : _data(std::make_unique<_parser_t>(sigma, stream, error)) { }

  parser::~parser() = default;

  std::optional<parser::result> parser::parse() {
    std::optional<formula> f = _data->parse_formula(false);
    if(!f)
      return {};

    if(_data->peek())
      return 
        _data->error("Expected end of formula, found " + 
          to_string(*_data->peek()));

    return {{*f, _data->_features}};
  }

  std::optional<token> parser::_parser_t::peek() {
    return _lex.peek();
  }

  std::optional<token> 
  parser::_parser_t::peek(token::type t, std::string const&err) {
    auto tok = peek();
    if(!tok || tok->token_type() != t)
      return error("Expected " + err);

    return tok;
  }

  std::optional<token> parser::_parser_t::consume() {
    auto tok = peek();
    if(tok) {
      set_features(*tok);
      _lex.get();
    }
    return tok;
  }

  std::optional<token> 
  parser::_parser_t::consume(token::type t, std::string const&err) {
    auto tok = peek(t, err);
    if(tok) {
      set_features(*tok);
      _lex.get();
    }
    return tok;
  }

  std::optional<token> 
  parser::_parser_t::consume_punctuation(token::punctuation p) {
    auto tok = peek();
    if(!tok || !tok->is<token::punctuation>() ||
        tok->data<token::punctuation>() != p) {
      return error("Expected '" + to_string(p) + "'");
    }
    if(tok) {
      set_features(*tok);
      _lex.get();
    }
    return tok;
  }

  std::nullopt_t parser::_parser_t::error(std::string const&s) {
    _error(s);
    return std::nullopt;
  }
  
  //
  // This function sets the detected feature of the formula based on the
  // scanned tokens.
  //
  void parser::_parser_t::set_features(token const& tok) 
  {
    if(auto k = tok.data<token::keyword>(); k) { // next, exists, forall
      _features |= feature::first_order;
      switch(*k){
        case token::keyword::next:
        case token::keyword::wnext:
          _features |= feature::nextvar;
          break;
        case token::keyword::exists:
          _features |= feature::quantifiers;
          break;
        case token::keyword::forall:
          _features |= feature::quantifiers;
          _features |= feature::forall;
      }
    }

    if(auto type = tok.data<binary::type>(); type) {
      if(to_underlying(*type) >= to_underlying(binary::type::until))
        _features |= feature::temporal;
      if(to_underlying(*type) >= to_underlying(binary::type::since))
        _features |= feature::past;
    }
    
    if(auto type = tok.data<unary::type>(); type) {
      switch(*type) {
        case unary::type::negation:
          break;
        case unary::type::yesterday:
        case unary::type::w_yesterday:
        case unary::type::once:
        case unary::type::historically: 
          _features |= feature::past;
          _features |= feature::temporal;
          break;
        case unary::type::tomorrow:
        case unary::type::w_tomorrow:
        case unary::type::always:
        case unary::type::eventually:
          _features |= feature::temporal;
          break;  
      }
    }
  }

  std::optional<formula> parser::_parser_t::parse_formula(bool quantified) {
    std::optional<formula> lhs = parse_primary(quantified);
    if(!lhs)
      return error("Expected formula");

    return parse_binary_rhs(0, *lhs, quantified);
  }

  std::optional<formula> parser::_parser_t::parse_binary_rhs(
    int prec, formula lhs, bool quantified
  ) {
    while(1) {
      if(!peek() || precedence(*peek()) < prec)
         return {lhs};

      token op = *consume();
      std::optional<binary::type> btype = op.data<binary::type>();

      if(quantified && 
         btype && to_underlying(*btype) >= to_underlying(binary::type::until))
        return 
          error("Temporal operators inside quantifiers are not supported.");

      std::optional<formula> rhs = parse_primary(quantified);
      if(!rhs)
        return error("Expected right operand to binary operator");

      if(!peek() || precedence(op) < precedence(*peek())) {
        rhs = parse_binary_rhs(prec + 1, *rhs, quantified);
        if(!rhs)
          return error("Expected right operand to binary operator");
      }

      black_assert(op.is<binary::type>());
      lhs = binary(*op.data<binary::type>(), lhs, *rhs);
    }
  }

  std::optional<formula> parser::_parser_t::parse_boolean()
  {
    black_assert(peek() && peek()->token_type() == token::type::boolean);

    std::optional<token> tok = consume();

    black_assert(tok);
    black_assert(tok->token_type() == token::type::boolean);

    return _alphabet.boolean(*tok->data<bool>());
  }

  std::optional<formula> parser::_parser_t::parse_atom()
  {
    std::optional<term> lhs = parse_term();
    if(!lhs)
      return {};

    // if there is no relation symbol after the term, 
    // the term was not a term after all, but a relational atom
    if(!peek() || !peek()->is<relation::type>())
      return correct_term_to_formula(*lhs);

    if(!register_term(*lhs))
      return {};

    // otherwise we parse the rhs and form the atom
    relation::type r = *peek()->data<relation::type>();
    consume();

    std::optional<term> rhs = parse_term();
    if(!rhs)
      return {};

    if(!register_term(*rhs))
      return {};
    
    _features |= feature::first_order;
    return atom(relation{r}, {*lhs, *rhs});
  }

  std::optional<formula> parser::_parser_t::parse_quantifier() {
    black_assert(peek());
    black_assert(
      peek()->data<token::keyword>() == token::keyword::exists ||
      peek()->data<token::keyword>() == token::keyword::forall
    );

    quantifier::type q = 
      consume()->data<token::keyword>() == token::keyword::exists ? 
      quantifier::type::exists : quantifier::type::forall;

    std::optional<token> vartok = consume(token::type::identifier, "variable");
    if(!vartok)
      return {};
        
    std::string varname{*vartok->data<std::string>()};

    std::optional<token> dot = consume();
    if(!dot || dot->data<token::punctuation>() != token::punctuation::dot)
      return error("Expected dot after quantifier");

    std::optional<formula> matrix = parse_formula(true);
    if(!matrix)
      return {};

    variable var = _alphabet.var(varname);

    return quantifier(q, var, *matrix);
  }

  std::optional<formula> parser::_parser_t::parse_unary(bool quantified)
  {
    std::optional<token> op = consume(); // consume unary op
    black_assert(op && op->is<unary::type>());

    if(quantified && op->data<unary::type>() != unary::type::negation)
      return error("Temporal operators inside quantifiers are not supported.");

    std::optional<formula> formula = parse_primary(quantified);
    if(!formula)
      return {};

    return unary(*op->data<unary::type>(), *formula);
  }

  std::optional<formula> parser::_parser_t::parse_parens(bool quantified) {
    black_assert(peek());
    black_assert(
      peek()->data<token::punctuation>() == token::punctuation::left_paren);

    consume(); // Consume left paren '('

    std::optional<formula> formula = parse_formula(quantified);
    if(!formula)
      return {}; // error raised by parse();

    if(!consume_punctuation(token::punctuation::right_paren))
      return {}; // error raised by consume()

    return formula;
  }

  std::optional<formula> parser::_parser_t::parse_primary(bool quantified) {
    if(!peek())
      return {};

    if(peek()->token_type() == token::type::boolean)
      return parse_boolean();
    if(peek()->token_type() == token::type::integer ||
       peek()->data<function::type>() == function::type::subtraction ||
       peek()->token_type() == token::type::identifier ||
       peek()->data<token::keyword>() == token::keyword::next ||
       peek()->data<token::keyword>() == token::keyword::wnext)
      return parse_atom();
    if(peek()->data<token::keyword>() == token::keyword::exists ||
       peek()->data<token::keyword>() == token::keyword::forall)
      return parse_quantifier();
    if(peek()->is<unary::type>())
      return parse_unary(quantified);
    if(peek()->data<token::punctuation>() == token::punctuation::left_paren)
       return parse_parens(quantified);

    return error("Expected formula");
  }

  std::optional<formula> parser::_parser_t::correct_term_to_formula(term t) {
    return t.match(
      [&](constant c) -> std::optional<formula> {
        std::string v;
        if(std::holds_alternative<int>(c.value()))
          v = std::to_string(std::get<int>(c.value()));
        else
          v = std::to_string(std::get<double>(c.value()));

        return error("Expected formula, found numeric constant: " + v);
      },
      [&](variable x) {
        return _alphabet.prop(x.label());
      },
      [&](application a) -> std::optional<formula> {
        if(a.func().known_type())
          return error("Expected formula, found term");
        _features |= feature::first_order;

        std::string id = a.func().name();

        if(auto it = _rel_arities.find(id); it != _rel_arities.end())
          if((size_t)it->second != a.arguments().size())
            return error(
              "Relation symbol '" + id + "' used twice with different arities"
            );

        if(auto it = _func_arities.find(id); it != _func_arities.end())
          return error(
            "Relation symbol '" + id + "' already used as a function symbol"
          );

        _rel_arities.insert({id, a.arguments().size()});
        return atom(relation{a.func().name()}, a.arguments());
      },
      [&](next) -> std::optional<formula> { 
        return error("Expected formula, found 'next' expression");
      },
      [&](wnext) -> std::optional<formula> { 
        return error("Expected formula, found 'wnext' expression");
      }
    );
  }

  std::optional<term> parser::_parser_t::register_term(term t) 
  { 
    return t.match(
      [&](application app) -> std::optional<term> {
        std::string id = app.func().name();

        if(auto it = _func_arities.find(id); it != _func_arities.end())
          if((size_t)it->second != app.arguments().size())
            return error(
              "Function symbol '" + id + "' used twice with different arities"
            );

        if(auto it = _rel_arities.find(id); it != _rel_arities.end())
          return error(
            "Function symbol '" + id + "' already used as a relation symbol"
          );
        _func_arities.insert({id, app.arguments().size()});

        return t;
      },
      [&](otherwise) -> std::optional<term> { return t; }
    );
  }

  std::optional<term> parser::_parser_t::parse_term() {
    std::optional<term> lhs = parse_term_primary();
    if(!lhs)
      return error("Expected term");
    
    return parse_term_binary_rhs(0, *lhs);
  }

  std::optional<term> parser::_parser_t::parse_term_primary() {
    if(!peek())
      return {};

    if(peek()->token_type() == token::type::integer ||
       peek()->token_type() == token::type::real)
      return parse_term_constant();

    if(peek()->data<function::type>() == function::type::subtraction)
      return parse_term_unary_minus();

    if(peek()->data<token::keyword>() == token::keyword::next)
       return parse_term_next();

    if(peek()->data<token::keyword>() == token::keyword::wnext)
       return parse_term_wnext();

    if(peek()->token_type() == token::type::identifier)
      return parse_term_var_or_func();

    if(peek()->data<token::punctuation>() == token::punctuation::left_paren)
      return parse_term_parens();

    return error("Expected term, found " + to_string(*peek()));
  }

  std::optional<term> 
  parser::_parser_t::parse_term_binary_rhs(int prec, term lhs) {
    while(1) {
      if(!peek() || func_precedence(*peek()) < prec)
         return {lhs};

      token op = *consume();

      std::optional<term> rhs = parse_term_primary();
      if(!rhs)
        return error("Expected right operand to binary function symbol");

      if(!register_term(lhs) || !register_term(*rhs))
        return {};

      if(!peek() || func_precedence(op) < func_precedence(*peek())) {
        rhs = parse_term_binary_rhs(prec + 1, *rhs);
        if(!rhs)
          return error("Expected right operand to binary function symbol");
      }
      
      black_assert(op.is<function::type>());
      lhs = application(function{*op.data<function::type>()}, {lhs, *rhs});
    }
  }

  std::optional<term> parser::_parser_t::parse_term_constant() {
    black_assert(peek());
    black_assert(
      peek()->token_type() == token::type::integer ||
      peek()->token_type() == token::type::real
    );

    token tok = *peek();
    consume();

    if(tok.token_type() == token::type::integer)
      return _alphabet.constant(*tok.data<int>());
    else
      return _alphabet.constant(*tok.data<double>());
  }

  std::optional<term> parser::_parser_t::parse_term_unary_minus() {
    black_assert(peek());
    black_assert(peek()->data<function::type>() == function::type::subtraction);

    consume();
    std::optional<term> t = parse_term();
    if(!t)
      return {};
    if(!register_term(*t))
      return {};
    return application(function{function::type::negation}, {*t});
  }

  std::optional<term> parser::_parser_t::parse_term_next() {
    black_assert(peek()->data<token::keyword>() == token::keyword::next);

    consume();

    if(!consume_punctuation(token::punctuation::left_paren))
      return {};

    std::optional<term> t = parse_term();
    if(!t)
      return {};

    if(!register_term(*t))
      return {};

    if(!consume_punctuation(token::punctuation::right_paren))
      return {};

    return next(*t);
  }

  std::optional<term> parser::_parser_t::parse_term_wnext() {
    black_assert(peek()->data<token::keyword>() == token::keyword::wnext);

    consume();

    if(!consume_punctuation(token::punctuation::left_paren))
      return {};

    std::optional<term> t = parse_term();
    if(!t)
      return {};

    if(!register_term(*t))
      return {};

    if(!consume_punctuation(token::punctuation::right_paren))
      return {};

    return wnext(*t);
  }

  std::optional<term> 
  parser::_parser_t::parse_term_var_or_func() {
    black_assert(peek());
    black_assert(peek()->token_type() == token::type::identifier);

    std::string id{*peek()->data<std::string>()};
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
      if(!register_term(*t))
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

  std::optional<term> parser::_parser_t::parse_term_parens() {
    black_assert(peek());
    black_assert(
      peek()->data<token::punctuation>() == token::punctuation::left_paren);

    consume(); // Consume left paren '('

    std::optional<term> t = parse_term();
    if(!t)
      return {}; // error raised by parse();

    if(!consume_punctuation(token::punctuation::right_paren))
      return {}; // error raised by consume()

    return t;
  }

} // namespace black::internal
