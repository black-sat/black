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

    if(cli::filename && cli::formula) {
      command_line_error(
        "please specify only either a filename or the --formula option"
      );
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

    if(cli::logic)
      sigma.set_logic(*logic_from_string(*cli::logic));

    std::optional<black::formula> f =
      black::parse_formula(sigma, file, formula_syntax_error_handler(path));

    black_assert(f.has_value());

    black::solver slv;

    if (cli::sat_backend)
      slv.set_sat_backend(*cli::sat_backend);

    if (cli::remove_past)
      slv.set_formula(black::remove_past(*f), cli::finite);
    else
      slv.set_formula(*f, cli::finite);

    size_t bound = 
      cli::bound ? *cli::bound : std::numeric_limits<size_t>::max();
    black::tribool res = slv.solve(bound);

    output(res, slv, *f);

    return 0;
  }

  static 
  void relevant_props(formula f, std::unordered_set<proposition> &props) 
  {
    using namespace black;
    f.match(
      [&](boolean) {},
      [&](atom) {},
      [&](proposition p) {
        props.insert(p);
      },
      [&](unary, formula f1) {
        relevant_props(f1, props);
      },
      [&](binary, formula f1, formula f2) {
        relevant_props(f1, props);
        relevant_props(f2, props);
      }
    );
  }

  static
  void readable(tribool result, solver &solver, formula f)
  {
    if(result == tribool::undef) {
      io::println("UNKNOWN (stopped at k = {})", solver.last_bound());
      return;
    }

    if(result == false) {
      io::println("UNSAT");
      return;
    }

    black_assert(solver.model().has_value());
    io::println("SAT");

    if(!cli::print_model)
      return;

    if(cli::finite)
      io::println("Finite model:");
    else
      io::println("Model:");

    std::unordered_set<proposition> props;
    relevant_props(f, props);
    
    size_t size = solver.model()->size();
    size_t width = static_cast<size_t>(log10((double)size)) + 1;
    for(size_t t = 0; t < size; ++t) {
      io::print("- t = {:>{}}: {{", t, width);
      bool first = true;
      for(proposition a : props) {
        tribool v = solver.model()->value(a, t);
        const char *comma = first ? "" : ", ";
        if(v == true) {
          io::print("{}{}", comma, to_string(a));
          first = false;
        } else if(v == false) {
          io::print("{}￢{}", comma, to_string(a));
          first = false;
        }
      }
      io::print("}}");
      if(solver.model()->loop() == t)
        io::print(" ⬅︎ loops here");
      io::print("\n");
    }
  }

  static
  void json(tribool result, solver &solver, formula f) {
    io::println("{{");
    
    io::println("    \"result\": \"{}\",", 
      result == tribool::undef ? "UNKNOWN" :
      result == true  ? "SAT" : "UNSAT"
    );

    io::println("    \"k\": {}{}", 
      solver.last_bound(),
      cli::print_model && result == true ? "," : ""
    );

    if(result == true && cli::print_model) {
      auto model = solver.model();
      std::unordered_set<proposition> props;
      relevant_props(f, props);

      io::println("    \"model\": {{");
      io::println("        \"size\": {},", model->size());
      if(!cli::finite)
        io::println("        \"loop\": {},", model->loop());

      io::println("        \"states\": [");

      for(size_t t = 0; t < model->size(); ++t) {
        io::println("            {{");

        size_t i = 0;
        for(proposition a : props) {
          tribool v = model->value(a, t);
          io::println("                \"{}\": \"{}\"{}",
            to_string(a),
            v == tribool::undef ? "undef" :
            v == true           ? "true" : "false",
            i < props.size() - 1 ? "," : ""
          );
          ++i;
        }

        io::println("            }}{}", t < model->size() - 1 ? "," : "");
      }

      io::println("        ]");

      io::println("    }}");
    }

    io::println("}}");
  }

  void output(tribool result, solver &solver, formula f) {
    if(cli::output_format == "json")
      return json(result, solver, f);

    return readable(result, solver, f);
  }

}
