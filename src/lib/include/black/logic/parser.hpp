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

#ifndef BLACK_PARSER_H_
#define BLACK_PARSER_H_

#include <black/support/common.hpp>
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
  class BLACK_EXPORT parser
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
    std::optional<formula> parse_proposition();
    std::optional<formula> parse_unary();
    std::optional<formula> parse_parens();
    std::optional<formula> parse_primary();

  private:
    alphabet &_alphabet;
    lexer _lex;
    std::function<void(std::string)> _error;
  };

  // Easy entry-point for parsing formulas
  BLACK_EXPORT
  std::optional<formula>
  parse_formula(alphabet &sigma, std::string const&s,
                parser::error_handler error);

  BLACK_EXPORT
  std::optional<formula>
  parse_formula(alphabet &sigma, std::istream &s,
                parser::error_handler error);

  BLACK_EXPORT
  inline std::optional<formula>
  parse_formula(alphabet &sigma, std::string const&s) {
    return parse_formula(sigma, s, [](auto){});
  }

  BLACK_EXPORT
  inline std::optional<formula>
  parse_formula(alphabet &sigma, std::istream &s) {
    return parse_formula(sigma, s, [](auto){});
  }

  BLACK_EXPORT
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
      {40}, // implication
      {40}, // iff
      {50}, // until
      {50}, // release
      {50}, // weak until
      {50}, // strong release
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
