//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#include <black/frontend/solve.hpp>

#include <black/frontend/io.hpp>
#include <black/frontend/cli.hpp>
#include <black/frontend/support.hpp>

#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/past_remover.hpp>
#include <black/solver/solver.hpp>

#include <sstream>

namespace black::frontend {

  void output(tribool result, solver &solver, formula f);
  
  int solve(std::optional<std::string> const&path, std::istream &file);

  int solve() {
    if(!cli::filename && !cli::formula) {
      command_line_error("please specify a filename or the --formula option");
      quit(status_code::command_line_error);
    }

    if(cli::formula) {
      std::istringstream str{*cli::formula};
      return solve(std::nullopt, str);
    }

    if(*cli::filename == "-")
      return solve(std::nullopt, std::cin);

    std::ifstream file = open_file(*cli::filename);
    return solve(cli::filename, file);
  }

  int solve(std::optional<std::string> const&path, std::istream &file)
  {
    black::alphabet sigma;

    std::optional<black::formula> f =
      black::parse_formula(sigma, file, formula_syntax_error_handler(path));

    black_assert(f.has_value());

    black::solver slv{sigma};

    if (cli::sat_backend)
      slv.set_sat_backend(*cli::sat_backend);

    if (cli::remove_past)
      slv.assert_formula(black::remove_past(*f));
    else
      slv.assert_formula(*f);

    size_t bound = 
      cli::bound ? *cli::bound : std::numeric_limits<size_t>::max();
    black::tribool res = slv.solve(bound);

    output(res, slv, *f);

    return 0;
  }

  static 
  void relevant_atoms(formula f, std::unordered_set<atom> &atoms) 
  {
    using namespace black;
    f.match(
      [&](boolean) {},
      [&](atom a) {
        atoms.insert(a);
      },
      [&](unary, formula f1) {
        relevant_atoms(f1, atoms);
      },
      [&](binary, formula f1, formula f2) {
        relevant_atoms(f1, atoms);
        relevant_atoms(f2, atoms);
      }
    );
  }

  static
  void readable(tribool result, solver &solver, formula f)
  {
    if(result == tribool::undef) {
      io::message("UNKNOWN (stopped at k = {})", solver.last_bound());
      return;
    }

    if(result == false) {
      io::message("UNSAT");
      return;
    }

    black_assert(solver.model().has_value());
    io::message("SAT");

    if(!cli::print_model)
      return;

    io::message("Model:", solver.model()->size());

    std::unordered_set<atom> atoms;
    relevant_atoms(f, atoms);
    
    size_t size = solver.model()->size();
    size_t width = static_cast<size_t>(log10(size)) + 1;
    for(size_t t = 0; t < size; ++t) {
      io::print(verbosity::message, "- t = {:>{}}: {{", t, width);
      bool first = true;
      for(atom a : atoms) {
        tribool v = solver.model()->value(a, t);
        const char *comma = first ? "" : ", ";
        if(v == true) {
          io::print(verbosity::message, "{}{}", comma, to_string(a));
          first = false;
        } else if(v == false) {
          io::print(verbosity::message, "{}￢{}", comma, to_string(a));
          first = false;
        }
      }
      io::print(verbosity::message, "}}");
      if(solver.model()->loop() == t)
        io::print(verbosity::message, " ⬅︎ loops here");
      io::print(verbosity::message, "\n");
    }
  }

  static
  void json(tribool result, solver &solver, formula f) {
    io::message("{{");
    
    io::message("    \"result\": \"{}\",", 
      result == tribool::undef ? "UNKNOWN" :
      result == true  ? "SAT" : "UNSAT"
    );

    io::message("    \"k\": {}{}", 
      solver.last_bound(),
      cli::print_model && result == true ? "," : ""
    );

    if(result == true && cli::print_model) {
      auto model = solver.model();
      std::unordered_set<atom> atoms;
      relevant_atoms(f, atoms);

      io::message("    \"model\": {{");
      io::message("        \"size\": {},", model->size());
      if(model->loop())
        io::message("        \"loop\": {},", model->loop());

      io::message("        \"states\": [");

      for(size_t t = 0; t < model->size(); ++t) {
        io::message("            {{");

        size_t i = 0;
        for(atom a : atoms) {
          tribool v = model->value(a, t);
          io::message("                \"{}\": \"{}\"{}",
            to_string(a),
            v == tribool::undef ? "undef" :
            v == true           ? "true" : "false",
            i < atoms.size() - 1 ? "," : ""
          );
          ++i;
        }

        io::message("            }}{}", t < model->size() - 1 ? "," : "");
      }

      io::message("        ]");

      io::message("    }}");
    }

    io::message("}}");
  }

  void output(tribool result, solver &solver, formula f) {
    if(!cli::output_format || cli::output_format == "readable")
      return readable(result, solver, f);
    
    if(cli::output_format == "json")
      return json(result, solver, f);
    
    black_unreachable();
  }

}
