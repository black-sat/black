//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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


#include <black/frontend/synth.hpp>
#include <black/frontend/cli.hpp>
#include <black/frontend/support.hpp>
#include <black/logic/logic.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>

#include <black/automata/automaton.hpp>

#include <sstream>
#include <iostream>

namespace black::frontend {

  int synth(std::optional<std::string> const&path, std::istream &file);

  int synth() {
    if(!cli::filename && !cli::formula) {
      command_line_error("please specify a filename or the --formula option");
      quit(status_code::command_line_error);
    }

    if(cli::filename && cli::formula) {
      command_line_error(
        "please specify only either a filename or the --formula option"
      );
      quit(status_code::command_line_error);
    }

    if(cli::formula) {
      std::istringstream str{*cli::formula};
      return synth(std::nullopt, str);
    }

    if(*cli::filename == "-")
      return synth(std::nullopt, std::cin);

    std::ifstream file = open_file(*cli::filename);
    return synth(cli::filename, file);
  }

  int synth(std::optional<std::string> const&path, std::istream &file)
  {
    black::alphabet sigma;

    auto parsed = 
      black::parse_formula(sigma, file, formula_syntax_error_handler(path));
    black_assert(parsed.has_value());

    if(!parsed->is<logic::formula<logic::LTLP>>()) {
      io::errorln(
        "{}: the `synth` command is supported only for propositional LTL+P "
        "formulas", cli::command_name
      );
      quit(status_code::syntax_error);
    }
    
    
    auto f = parsed->to<logic::formula<logic::LTLP>>().value();

    io::println("formula: {}", to_string(f));

    return 0;
  }

}