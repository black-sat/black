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
#include <black/synth/synth.hpp>
#include <black/synth/fixpoint.hpp>

#include <sstream>
#include <iostream>
#include <filesystem>

namespace black::frontend {

  int synth(std::string const&path, std::istream &file, std::istream& partfile);
  int synth(std::istream &file, std::vector<std::string> inputs);

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
      return synth(str, cli::inputs);
    }

    std::filesystem::path filepath{*cli::filename};
    std::filesystem::path partpath{*cli::filename};
    if(partpath.extension() == ".ltlf")
      partpath.replace_extension("part");
    else
      partpath += ".part";

    std::ifstream file = open_file(filepath.string());
    std::ifstream part = open_file(partpath.string());
    return synth(filepath.string(), file, part);
  }

  int synth(std::string const&path, std::istream &file, std::istream& partfile)
  {
    black::alphabet sigma;

    auto spec = parse_ltlp_spec(
      sigma, file, partfile, formula_syntax_error_handler(path)
    );

    black::sdd::manager manager{&sigma, 1};

    automata_spec autspec = to_automata_spec(&manager, *spec);

    black::tribool result = is_realizable(autspec);

    if(result)
      io::println("REALIZABLE\n");
    else
      io::println("UNREALIZABLE\n");

    return 0;
  }
  
  int synth(std::istream &file, std::vector<std::string> inputs) {
    black::alphabet sigma;

    auto error = formula_syntax_error_handler({});
    auto parsed = parse_formula(sigma, file, lexer::syntax::spin, error);
    if(!parsed->is<black::logic::formula<black::logic::LTLP>>()) {
      error("synthesis is only supported for LTL+P formulas");
      return {};
    }

    auto f = *parsed->to<black::logic::formula<black::logic::LTLP>>();

    ltlp_spec spec = { {}, {}, f };
    
    for(auto str : inputs)
      spec.inputs.push_back(sigma.proposition(str));

    std::unordered_set<black::proposition> outputset;
    transform(f, [&](auto child) {
      child.match(
        [&](proposition p) {
          if(
            std::find(begin(spec.inputs), end(spec.inputs), p) == 
            end(spec.inputs)
          ) {
            outputset.insert(p);
          }
        },
        [](otherwise) { }
      );
    });

    spec.outputs.insert(end(spec.outputs), begin(outputset), end(outputset));

    black::sdd::manager manager{&sigma, 1};

    automata_spec autspec = to_automata_spec(&manager, spec);

    black_assert(autspec.spec.manager);

    black::tribool result = is_realizable(autspec);

    if(result)
      io::println("REALIZABLE\n");
    else
      io::println("UNREALIZABLE\n");

    return 0;
  }

}