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

#include <black/logic/parser.hpp>
#include <black/logic/lex.hpp>
//#include <black/logic/past_remover.hpp>

#include <tsl/hopscotch_map.h>

#include <string>
#include <vector>
#include <sstream>

namespace black_internal
{
  using namespace black;

  // Easy entry-point for parsing formulas
  std::optional<formula>
  parse_formula(
    alphabet &sigma, std::string const&s, parser::error_handler error
  ) {
    std::stringstream stream{s, std::stringstream::in};
    parser p{sigma, stream, std::move(error)};

    return p.parse();
  }

  // Easy entry-point for parsing formulas
  std::optional<formula>
  parse_formula(
    alphabet &sigma, std::istream &stream, parser::error_handler error
  ) {
    parser p{sigma, stream, std::move(error)};

    return p.parse();
  }

  struct parser::_parser_t {
    alphabet &_alphabet;
    lexer _lex;
    bool _trying = false;
    std::function<void(std::string)> _error;
    std::vector<token> _tokens;
    size_t _pos = 0;

    _parser_t(alphabet &sigma, std::istream &stream, error_handler error);

    template<typename F>
    auto try_parse(F f);

    std::optional<token> peek();
    std::optional<token> get();
    std::optional<token> consume();
    std::optional<token> consume(token::punctuation p);

    std::nullopt_t error(std::string const&s);

    std::optional<formula> parse_formula();
    std::optional<formula> parse_binary_rhs(int, formula);
    std::optional<formula> parse_boolean();
    std::optional<formula> parse_relational_atom();
    std::optional<formula> parse_atom();
    std::optional<formula> parse_quantifier();
    std::optional<formula> parse_unary();
    std::optional<formula> parse_modality();
    std::optional<formula> parse_parens();
    std::optional<formula> parse_primary();

    std::optional<sort> parse_sort();

    std::optional<term> build_binary_term(binary_term::type, term, term);
    std::optional<term> parse_term();
    std::optional<term> parse_term_primary();
    std::optional<term> parse_term_binary_rhs(int precedence, term lhs);
    std::optional<term> parse_term_constant();
    std::optional<term> parse_term_unary_minus();
    std::optional<term> parse_term_ctor();
    std::optional<term> parse_term_var_or_func();
    std::optional<term> parse_term_parens();
  };

  parser::parser(alphabet &sigma, std::istream &stream, error_handler error)
    : _data(std::make_unique<_parser_t>(sigma, stream, error)) { }

  parser::~parser() = default;

  std::optional<formula> parser::parse() {
    std::optional<formula> f = _data->parse_formula();
    if(!f)
      return {};

    if(_data->peek())
      return 
        _data->error("Expected end of formula, found '" + 
          to_string(*_data->peek()) + "'");

    return *f;
  }

  parser::_parser_t::_parser_t(
    alphabet &sigma, std::istream &stream, error_handler error
  ) : _alphabet(sigma), _lex(stream, error), _error(error)
  {
    std::optional<token> tok = _lex.get();
    if(tok)    
      _tokens.push_back(*tok);
  }

  template<typename F>
  auto parser::_parser_t::try_parse(F f) {
    size_t pos = _pos;
    
    bool t = _trying;
    _trying = true;
    
    auto r = f();
    if(!r)
      _pos = pos;
    
    _trying = t;
    return r;
  }

  std::optional<token> parser::_parser_t::peek() {
    if(_pos < _tokens.size())
      return _tokens[_pos];
    return {};
  }

  std::optional<token> parser::_parser_t::get() {
    _pos++;
    if(_pos < _tokens.size())
      return _tokens[_pos];
    
    std::optional<token> tok = _lex.get();
    if(!tok)
      return {};
    
    _tokens.push_back(*tok);
    return tok;
  }

  std::optional<token> parser::_parser_t::consume() {
    auto tok = peek();
    if(tok)
      get();

    return tok;
  }

  std::optional<token> 
  parser::_parser_t::consume(token::punctuation p) {
    auto tok = peek();
    if(!tok || !tok->is<token::punctuation>() ||
        tok->data<token::punctuation>() != p) {
      if(tok)
        return error( // LCOV_EXCL_LINE
          "Expected '" + to_string(p) + "', found '" + to_string(*tok) + "'"
        );
      else
        return error( // LCOV_EXCL_LINE
          "Expected '" + to_string(p) + "', found end of input"
        );
    }
    consume();
    return tok;
  }

  std::nullopt_t parser::_parser_t::error(std::string const&s) {
    if(!_trying) {
      _error(s);
    }
    return std::nullopt;
  }

  std::optional<formula> parser::_parser_t::parse_formula() {
    std::optional<formula> lhs = parse_primary();
    if(!lhs)
      return error("Expected formula");

    return parse_binary_rhs(0, *lhs);
  }

  static std::optional<int> precedence(token const&tok)
  {
    if(!tok.data<binary::type>())
      return {};
    
    switch(*tok.data<binary::type>()) {
      case binary::type::conjunction:
        return 30;
      case binary::type::disjunction:
        return 20;
      case binary::type::implication:
        return 40;
      case binary::type::iff:        
        return 40;
      case binary::type::until:      
        return 50;
      case binary::type::release:    
        return 50;
      case binary::type::w_until:    
        return 50;
      case binary::type::s_release:  
        return 50;
      case binary::type::since:      
        return 50;
      case binary::type::triggered:  
        return 50;
    }
    black_unreachable();
  }

  std::optional<formula> 
  parser::_parser_t::parse_binary_rhs(int prec, formula lhs) {
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

  std::optional<formula> parser::_parser_t::parse_boolean()
  {
    black_assert(peek() && peek()->token_type() == token::type::boolean);  // LCOV_EXCL_LINE

    std::optional<token> tok = consume();

    black_assert(tok);
    black_assert(tok->token_type() == token::type::boolean);

    return _alphabet.boolean(*tok->data<bool>());
  }

  std::optional<formula> parser::_parser_t::parse_relational_atom() {
    black_assert(peek());
    
    std::optional<term> lhs = parse_term();
    if(!lhs)
      return {};

    if(!peek() || 
      (peek()->token_type() != token::type::equality &&
       peek()->token_type() != token::type::comparison))
      return {};

    token op = *peek();
    consume();

    std::optional<term> rhs = parse_term();
    if(!rhs)
      return {};

    if(op.token_type() == token::type::equality)
      return equality(
        op.data<token::equality_t>()->first, std::vector<term>{*lhs, *rhs}
      );

    black_assert(op.token_type() == token::type::comparison);
    return comparison(*op.data<comparison::type>(), *lhs, *rhs);
  }

  std::optional<formula> parser::_parser_t::parse_atom()
  {
    black_assert(peek());

    std::optional<formula> a = 
      try_parse([&](){ return parse_relational_atom(); });
    if(a)
      return a;

    if(peek()->token_type() != token::type::identifier)
      return error("Expected identifier, found '" + to_string(*peek()) + "'");

    std::string id{*peek()->data<std::string>()};
    consume();

    // if there is no open paren this is a simple proposition
    if(!peek() || 
        peek()->data<token::punctuation>() != token::punctuation::left_paren)
      return _alphabet.proposition(id);

    // otherwise it is a relational atom
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

    if(!consume(token::punctuation::right_paren))
      return {};

    relation r = _alphabet.relation(id);
    return r(terms);
  }

  std::optional<formula> parser::_parser_t::parse_quantifier() {
    black_assert(peek());
    black_assert(peek()->data<quantifier::type>());

    quantifier::type q = quantifier::type::forall;
    if(consume()->data<quantifier::type>() == quantifier::type::exists) 
      q = quantifier::type::exists;

    std::vector<var_decl> vars;
    while(
      !peek() || 
      peek()->data<token::punctuation>() != token::punctuation::dot
    ) {
      bool paren = false;
      if(peek()->data<token::punctuation>() == 
         token::punctuation::left_paren) 
      {
        consume();
        paren = true;
      }
      if(!peek())
        return error("Expected variable name, found end of input");

      if(peek()->token_type() != token::type::identifier)
        return 
          error("Expected variable name, found '" + to_string(*peek()) + "'");

      identifier varid = *consume()->data<std::string>();

      std::optional<sort> s;
      if(!peek())
        return error("Expected ':', found end of input");

      if(peek()->data<token::punctuation>() != token::punctuation::colon) 
        return error("Expected ':', found '" + to_string(*peek()) + "'");

      consume(); // consume the colon
      
      s = parse_sort();
      if(!s)
        return {};

      vars.push_back(_alphabet.var_decl(_alphabet.variable(varid), *s));

      if(paren && !consume(token::punctuation::right_paren))
        return {};

      if(
        peek() && 
        peek()->data<token::punctuation>() == token::punctuation::comma
      ) {
        consume();
        if(
          peek() && 
          peek()->data<token::punctuation>() == token::punctuation::comma
        ) {
          return error("Expected variable, found '" + to_string(*peek()) + ";");
        } 
      }
    }

    if(!peek())
      return error("Expected '.', found end of input");

    if(vars.empty())
      return error("Expected variable list, found '.'");

    consume(); // consume the dot

    std::optional<formula> matrix = parse_primary();
    if(!matrix)
      return {};

    if(q == quantifier::type::exists)
      return exists(vars, *matrix);

    return forall(vars, *matrix);
  }

  std::optional<formula> parser::_parser_t::parse_unary()
  {
    std::optional<token> op = consume(); // consume unary op
    black_assert(op && op->is<unary::type>());  // LCOV_EXCL_LINE

    std::optional<formula> formula = parse_primary();
    if(!formula)
      return {};

    return unary(*op->data<unary::type>(), *formula);
  }

  std::optional<formula> parser::_parser_t::parse_modality()
  {
    std::optional<token> op = consume(); // consume '<' or '['
    
    std::optional<term> standpoint;

    if(peek()->data<binary_term::type>() == binary_term::type::multiplication)
      standpoint = _alphabet.star();
    
    if(peek()->token_type() == token::type::identifier)
      standpoint = _alphabet.variable(*peek()->data<std::string>());

    if(!standpoint)
      return 
        error(
          "Expected standpoint (either '*' or named), found '" + 
          to_string(*peek()) + 
          "'"
        );
    consume();

    bool is_box = true;
    if(op->data<comparison::type>() == comparison::type::less_than) {
      is_box = false;
      
      if(!peek()) 
        return error("Expected '>', found end of input");

      if(peek()->data<comparison::type>() != comparison::type::greater_than)
        return error("Expected '>', found'" + to_string(*peek()) + "'");
    }
    
    if(op->data<token::punctuation>() == token::punctuation::left_bracket) {
      if(!peek()) 
        return error("Expected ']', found end of input");

      if(peek()->data<token::punctuation>()!= token::punctuation::right_bracket)
        return error("Expected ']', found'" + to_string(*peek()) + "'");
    }

    consume();

    std::optional<formula> formula = parse_primary();
    if(!formula)
      return {};

    if(is_box)
      return box(*standpoint, *formula);
    return diamond(*standpoint, *formula);
  }

  std::optional<formula> parser::_parser_t::parse_parens() {
    black_assert(peek());
    black_assert(
      peek()->data<token::punctuation>() == token::punctuation::left_paren);

    std::optional<formula> a = 
      try_parse([&](){ return parse_relational_atom(); });
    if(a)
      return a;

    consume(); // Consume left paren '('

    std::optional<formula> formula = parse_formula();
    if(!formula)
      return {}; // error raised by parse();

    if(!consume(token::punctuation::right_paren))
      return {}; // error raised by consume()

    return formula;
  }

  std::optional<formula> parser::_parser_t::parse_primary() {
    if(!peek())
      return {};

    if(peek()->token_type() == token::type::boolean)
      return parse_boolean();
    if(peek()->token_type() == token::type::integer ||
       peek()->token_type() == token::type::real ||
       peek()->data<binary_term::type>() == binary_term::type::subtraction ||
       peek()->token_type() == token::type::identifier ||
       peek()->data<unary_term::type>() == unary_term::type::to_integer ||
       peek()->data<unary_term::type>() == unary_term::type::to_real ||
       peek()->data<unary_term::type>() == unary_term::type::next ||
       peek()->data<unary_term::type>() == unary_term::type::wnext ||
       peek()->data<unary_term::type>() == unary_term::type::prev ||
       peek()->data<unary_term::type>() == unary_term::type::wprev)
      return parse_atom();
    if(peek()->data<quantifier::type>() == quantifier::type::exists ||
       peek()->data<quantifier::type>() == quantifier::type::forall)
      return parse_quantifier();
    if(peek()->is<unary::type>())
      return parse_unary();
    if(peek()->data<comparison::type>() == comparison::type::less_than ||
       peek()->data<token::punctuation>() == token::punctuation::left_bracket)
      return parse_modality();
    if(peek()->data<token::punctuation>() == token::punctuation::left_paren)
       return parse_parens();

    return error("Expected formula, found '" + to_string(*peek()) + "'");
  }

  std::optional<sort> parser::_parser_t::parse_sort() 
  {
    std::optional<token> tok = consume();

    if(!tok)
      return error("Expected sort, found end of input");

    if(tok->data<arithmetic_sort::type>() == 
       arithmetic_sort::type::integer_sort)
      return _alphabet.integer_sort();

    if(tok->data<arithmetic_sort::type>() == 
       arithmetic_sort::type::real_sort)
      return _alphabet.real_sort();

    if(tok->token_type() == token::type::identifier)
      return _alphabet.named_sort(*tok->data<std::string>());

    return error("Expected sort, found '" + to_string(*tok) + "'");
  }

  std::optional<term> parser::_parser_t::parse_term() {
    std::optional<term> lhs = parse_term_primary();
    if(!lhs) {
      if(peek())
        return error("Expected term, found '" + to_string(*peek()) + "'");
      return error("Expected term, found end of input");
    }
    
    return parse_term_binary_rhs(0, *lhs);
  }

  std::optional<term> parser::_parser_t::parse_term_primary() {
    if(!peek())
      return {};

    if(peek()->token_type() == token::type::integer ||
       peek()->token_type() == token::type::real)
      return parse_term_constant();

    if(peek()->data<binary_term::type>() == binary_term::type::subtraction)
      return parse_term_unary_minus();

    if(peek()->data<unary_term::type>() == unary_term::type::to_integer ||
       peek()->data<unary_term::type>() == unary_term::type::to_real ||
       peek()->data<unary_term::type>() == unary_term::type::next ||
       peek()->data<unary_term::type>() == unary_term::type::wnext ||
       peek()->data<unary_term::type>() == unary_term::type::prev ||
       peek()->data<unary_term::type>() == unary_term::type::wprev)
       return parse_term_ctor();

    if(peek()->token_type() == token::type::identifier)
      return parse_term_var_or_func();

    if(peek()->data<token::punctuation>() == token::punctuation::left_paren)
      return parse_term_parens();

    return error("Expected term, found '" + to_string(*peek()) + "'");
  }

  static std::optional<int> func_precedence(token const&tok)
  {
    if(!tok.data<binary_term::type>())
      return {};

    switch(*tok.data<binary_term::type>()) {
      case binary_term::type::addition:      
        return 20;
      case binary_term::type::subtraction:   
        return 20;
      case binary_term::type::multiplication:
        return 30;
      case binary_term::type::division:      
        return 30;
      case binary_term::type::int_division:  
        return 30;
    }
    black_unreachable();
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

      if(!peek() || func_precedence(op) < func_precedence(*peek())) {
        rhs = parse_term_binary_rhs(prec + 1, *rhs);
        if(!rhs)
          return error("Expected right operand to binary function symbol");
      }
      
      black_assert(op.is<binary_term::type>());
      lhs = binary_term(*op.data<binary_term::type>(), lhs, *rhs);
    }
  }

  std::optional<term> parser::_parser_t::parse_term_constant() {
    black_assert(peek());
    black_assert( // LCOV_EXCL_LINE
      peek()->token_type() == token::type::integer ||
      peek()->token_type() == token::type::real
    );

    token tok = *peek();
    consume();

    if(tok.token_type() == token::type::integer) {
      int64_t value = *tok.data<int64_t>();
      return constant(_alphabet.integer(value));
    }
    
    black_assert(tok.token_type() == token::type::real);

    double value = *tok.data<double>();
    return constant(_alphabet.real(value));
  }

  std::optional<term> parser::_parser_t::parse_term_unary_minus() {
    black_assert(peek());
    black_assert(
      peek()->data<binary_term::type>() == binary_term::type::subtraction
    );

    consume();
    std::optional<term> t = parse_term();
    if(!t)
      return {};
    return negative(*t);
  }

  std::optional<term> parser::_parser_t::parse_term_ctor() {
    black_assert(
      peek()->data<unary_term::type>() == unary_term::type::to_integer ||
      peek()->data<unary_term::type>() == unary_term::type::to_real ||
      peek()->data<unary_term::type>() == unary_term::type::next ||
      peek()->data<unary_term::type>() == unary_term::type::wnext ||
      peek()->data<unary_term::type>() == unary_term::type::prev ||
      peek()->data<unary_term::type>() == unary_term::type::wprev
    );

    token op = *consume();

    if(!consume(token::punctuation::left_paren))
      return {};

    std::optional<term> t = parse_term();
    if(!t)
      return {};

    if(!consume(token::punctuation::right_paren))
      return {};

    return unary_term(*op.data<unary_term::type>(), *t);
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
      return _alphabet.variable(id);

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

    if(!consume(token::punctuation::right_paren))
      return {};

    function f = _alphabet.function(id);
    return f(terms);
  }

  std::optional<term> parser::_parser_t::parse_term_parens() {
    black_assert(peek());
    black_assert(
      peek()->data<token::punctuation>() == token::punctuation::left_paren);

    consume(); // Consume left paren '('

    std::optional<term> t = parse_term();
    if(!t)
      return {}; // error raised by parse();

    if(!consume(token::punctuation::right_paren))
      return {}; // error raised by consume()

    return t;
  }

} // namespace black_internal
