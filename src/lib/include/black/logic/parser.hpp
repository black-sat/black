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
#include <sstream>
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

    parser(logic::alphabet &sigma, std::istream &stream, error_handler error);
    
    ~parser();

    std::optional<logic::formula<logic::Everything>> parse();

  private:
    struct _parser_t;
    std::unique_ptr<_parser_t> _data;
  };

  // Easy entry-point for parsing formulas
  template<black::logic::fragment Syntax>
  std::optional<black::logic::formula<Syntax>>
  parse_formula(
    black::logic::alphabet &sigma, 
    std::istream &stream, parser::error_handler error
  ) {
    parser p{sigma, stream, error};

    auto parsed = p.parse();
    if(!parsed)
      return std::nullopt;
    
    auto casted = parsed->to<black::logic::formula<Syntax>>();
    if(!casted) {
      error("unsupported syntax in parsed formula");
      return std::nullopt;
    }
    
    return casted;
  }

  template<black::logic::fragment Syntax>
  std::optional<black::logic::formula<Syntax>>
  parse_formula(
    black::logic::alphabet &sigma, 
    std::string const&s, parser::error_handler error
  ) {
    std::stringstream stream{s, std::stringstream::in};
    return parse_formula<Syntax>(sigma, stream, std::move(error));
  }

  template<black::logic::fragment Syntax>
  inline std::optional<logic::formula<Syntax>>
  parse_formula(logic::alphabet &sigma, std::string const&s) {
    return parse_formula<Syntax>(sigma, s, [](auto){});
  }

  template<black::logic::fragment Syntax>
  inline std::optional<logic::formula<Syntax>>
  parse_formula(logic::alphabet &sigma, std::istream &s) {
    return parse_formula<Syntax>(sigma, s, [](auto){});
  }

} // namespace black_internal

// Exported names
namespace black {
  using black_internal::parser;
  using black_internal::parse_formula;
  using black_internal::to_string;
}

#endif // PARSER_H_
