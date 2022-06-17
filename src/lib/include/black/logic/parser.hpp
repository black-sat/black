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
#include <black/logic/logic.hpp>
#include <black/logic/lex.hpp>

#include <istream>
#include <ostream>
#include <functional>
#include <memory>

namespace black::internal
{
  //
  // Class to parse LTL formulas
  //
  class BLACK_EXPORT parser
  {
  public:
    using error_handler = std::function<void(std::string)>;

    parser(alphabet &sigma, std::istream &stream, error_handler error);
    ~parser();

    std::optional<formula> parse();

  private:
    struct _parser_t;
    std::unique_ptr<_parser_t> _data;
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

  inline std::optional<int> precedence(token const&tok)
  {
    if(!tok.data<binary::type>())
      return {};
    
    return tok.data<binary::type>()->match(
      [](type_value<syntax_element::conjunction>) { return 30; },
      [](type_value<syntax_element::disjunction>) { return 20; },
      [](type_value<syntax_element::implication>) { return 40; },
      [](type_value<syntax_element::iff>)         { return 40; },
      [](type_value<syntax_element::until>)       { return 50; },
      [](type_value<syntax_element::release>)     { return 50; },
      [](type_value<syntax_element::w_until>)     { return 50; },
      [](type_value<syntax_element::s_release>)   { return 50; },
      [](type_value<syntax_element::since>)       { return 50; },
      [](type_value<syntax_element::triggered>)   { return 50; }
    );
  }

  inline std::optional<int> func_precedence(token const&tok)
  {
    if(!tok.data<binary_term::type>())
      return {};

    return tok.data<binary_term::type>()->match(
      [](type_value<syntax_element::addition>)       { return 20; },
      [](type_value<syntax_element::subtraction>)    { return 20; },
      [](type_value<syntax_element::multiplication>) { return 30; },
      [](type_value<syntax_element::division>)       { return 30; }
    );
  }

} // namespace black::internal

// Exported names
namespace black {
  using internal::parser;
  using internal::parse_formula;
  using internal::to_string;
}

#endif // PARSER_H_
