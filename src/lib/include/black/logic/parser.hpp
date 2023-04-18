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

namespace black_internal
{
  //
  // Class to parse LTL formulas
  //
  class BLACK_EXPORT parser
  {
  public:
    using error_handler = std::function<void(std::string)>;

    parser(
      logic::alphabet &sigma, std::istream &stream, lexer::syntax s, 
      error_handler error
    );
    parser(logic::alphabet &sigma, std::istream &stream, error_handler error);
    
    ~parser();

    std::optional<logic::formula<logic::LTLPFO>> parse();

  private:
    struct _parser_t;
    std::unique_ptr<_parser_t> _data;
  };

  // Easy entry-point for parsing formulas
  BLACK_EXPORT
  std::optional<logic::formula<logic::LTLPFO>>
  parse_formula(
    logic::alphabet &sigma, std::string const&s, parser::error_handler error
  );

  BLACK_EXPORT
  std::optional<logic::formula<logic::LTLPFO>>
  parse_formula(
    logic::alphabet &sigma, std::istream &s, parser::error_handler error
  );

  BLACK_EXPORT
  std::optional<logic::formula<logic::LTLPFO>>
  parse_formula(
    logic::alphabet &sigma, std::istream &s, lexer::syntax syntax,
    parser::error_handler error
  );

  BLACK_EXPORT
  inline std::optional<logic::formula<logic::LTLPFO>>
  parse_formula(logic::alphabet &sigma, std::string const&s) {
    return parse_formula(sigma, s, [](auto){});
  }

  BLACK_EXPORT
  inline std::optional<logic::formula<logic::LTLPFO>>
  parse_formula(logic::alphabet &sigma, std::istream &s) {
    return parse_formula(sigma, s, [](auto){});
  }

} // namespace black_internal

// Exported names
namespace black {
  using black_internal::parser;
  using black_internal::parse_formula;
  using black_internal::to_string;
}

#endif // PARSER_H_
