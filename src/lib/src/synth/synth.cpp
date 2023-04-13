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
//

#include <black/synth/synth.hpp>

#include <black/logic/lex.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>

#include <iostream>
#include <sstream>

namespace black_internal::synth {

  std::optional<ltlp_spec> parse_ltlp_spec(
    logic::alphabet &sigma, std::istream &fstr, std::istream &partstr,
    parser::error_handler error
  ) {
    auto parsed = parse_formula(sigma, fstr, error);
    if(!parsed->is<black::logic::formula<black::logic::LTLP>>()) {
      error("synthesis is only supported for LTL+P formulas");
      return {};
    }

    auto spec = *parsed->to<black::logic::formula<black::logic::LTLP>>();

    std::vector<black::proposition> inputs;
    std::vector<black::proposition> outputs;

    using black_internal::lexer;
    using black_internal::token;

    lexer lex{partstr, error};

    bool inputsdone = false;
    bool outputsdone = false;

    lex.get();
    while(!inputsdone || !outputsdone) {
      auto tok = lex.peek();
      if(tok->data<token::punctuation>() != token::punctuation::dot) {
        error("expected '.', found '" + to_string(*tok) + "'");
        return {};
      }
      
      std::vector<black::proposition> *target = nullptr;
      lex.get();
      if(lex.peek() && lex.peek()->data<std::string>() == "inputs") {
        inputsdone = true;
        target = &inputs;
      } else if(lex.peek() && lex.peek()->data<std::string>() == "outputs") {
        outputsdone = true;
        target = &outputs;
      } else {
        error(
          "expected .inputs or .outputs, found: ." + to_string(*lex.peek())
        );
        return {};
      }

      lex.get();
      if(
        !lex.peek() || 
        lex.peek()->data<token::punctuation>() != token::punctuation::colon
      ) {
        error("expected ':'");
        return {}; 
      }

      while(lex.get()) {        
        if(!lex.peek()->data<std::string>())
          break;
        
        target->push_back(sigma.proposition(*lex.peek()->data<std::string>()));
        
      }
    }

    return ltlp_spec{
      .inputs = inputs,
      .outputs = outputs,
      .spec = spec
    };
  }

  std::string to_string(ltlp_spec const& spec) {
    std::stringstream str;
    
    str << "spec parsed:\n";
    str << "- formula: " <<  to_string(spec.spec) << "\n";
    str << "- inputs:\n";
    for(auto p : spec.inputs)
      str << "  - " << to_string(p) << "\n";
    str << "- outputs:\n";
    for(auto p : spec.outputs)
      str << "  - " << to_string(p) << "\n";

    return str.str();
  }

}