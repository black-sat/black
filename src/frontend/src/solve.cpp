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

#include <black/support/config.hpp>

#include <black/frontend/io.hpp>
#include <black/frontend/cli.hpp>
#include <black/frontend/support.hpp>

#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/logic/past_remover.hpp>
#include <black/solver/solver.hpp>
#include <black/solver/core.hpp>
#include <black/sat/solver.hpp>

#include <iostream>
#include <sstream>

namespace black::frontend {

  void output(
    tribool result, solver &solver, formula f, std::optional<formula> muc
  );
  
  int solve(std::optional<std::string> const&path, std::istream &file);

  void trace(black::solver::trace_t data);

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

    if(cli::domain)
      sigma.set_domain(cli::domain == "integers" ? sort::Int : sort::Real);

    std::optional<formula> f =
      black::parse_formula(sigma, file, formula_syntax_error_handler(path));

    black_assert(f.has_value());

    uint8_t features = formula_features(*f);

    std::string backend = BLACK_DEFAULT_BACKEND;

    if (cli::sat_backend)
      backend = *cli::sat_backend;

    black_assert(black::sat::solver::backend_exists(backend));

    if(features & feature_t::first_order)
      cli::finite = true;

    if((features & feature_t::first_order) && 
       !black::sat::solver::backend_has_feature(backend, 
          black::sat::feature::smt))
    {
      io::errorln(
        "{}: the `{}` backend does not support first-order formulas.",
        cli::command_name, backend
      );
      quit(status_code::failure);
    }

    if((features & feature_t::quantifiers) && 
       !black::sat::solver::backend_has_feature(backend, 
          black::sat::feature::quantifiers))
    {
      io::errorln(
        "{}: the `{}` backend does not support "
        "quantified first-order formulas.",
        cli::command_name, backend
      );
      quit(status_code::failure);
    }

    if(!cli::domain && (features & feature_t::first_order)) {
      command_line_error(
        "the --domain option is required for first-order formulas."
      );
      quit(status_code::command_line_error);
    }

    if(cli::print_model && (features & feature_t::first_order)) {
      command_line_error(
        "model extraction is not supported (yet) for first-order formulas."
      );
      quit(status_code::command_line_error);
    }

    if(cli::unsat_core && (features & feature_t::first_order)) {
      command_line_error(
        "unsat core extraction is not supported for first-order formulas."
      );
      quit(status_code::command_line_error);
    }

    if(!cli::semi_decision && (features & feature_t::nextvar)) {
      cli::semi_decision = true;
      io::errorln(
      "{0}: warning: use of `next`/`prev` terms implies the --semi-decision "
           "option.\n"
      "{0}: warning: execution may not terminate.\n"
      "{0}: warning: pass the --semi-decision option explicitly to silence "
      "this warning.", cli::command_name
      );
    }

    if(cli::remove_past && (features & feature_t::first_order)) {
      command_line_error(
        "the --remove-past option is not supported for first-order formulas"
      );
      quit(status_code::command_line_error);
    }

    if(cli::debug == "print")
      io::println(
        "{}: debug: parsed formula: {}", cli::command_name, to_string(*f)
      );

    [[maybe_unused]]
    bool ok = 
      black::solver::check_syntax(*f, formula_syntax_error_handler(path));
    
    black_assert(ok); // the error handler quits

    black::solver slv;

    slv.set_sat_backend(backend);

    if(!cli::debug.empty())
      slv.set_tracer(&trace);

    if (cli::remove_past)
      slv.set_formula(black::remove_past(*f), cli::finite);
    else
      slv.set_formula(*f, cli::finite);

    size_t bound = 
      cli::bound ? *cli::bound : std::numeric_limits<size_t>::max();
    black::tribool res = slv.solve(bound, cli::semi_decision);

    std::optional<formula> muc;
    if(res == false && cli::unsat_core) {
      muc = unsat_core(*f, cli::finite);
    }

    output(res, slv, *f, muc);

    return 0;
  }

  static 
  void relevant_props(formula f, std::unordered_set<proposition> &props) 
  {
    using namespace black;
    f.match(
      [&](boolean) {},
      [&](atom) { black_unreachable(); }, // LCOV_EXCL_LINE
      [&](quantifier) { black_unreachable(); }, // LCOV_EXCL_LINE
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
  void print_uc_replacements(formula f, size_t &last_index) {
    f.match(
      [](boolean) { },
      [&](proposition p) {
        if(auto l = p.label<core_placeholder_t>(); l.has_value()) {
          if(l->n >= last_index) {
            io::print(" - {{{}}}: {}\n", l->n, l->f);
            last_index++;
          }
        }
      },
      [&](unary, formula arg) {
        print_uc_replacements(arg, last_index);
      },
      [&](binary, formula left, formula right) {
        print_uc_replacements(left, last_index);
        print_uc_replacements(right, last_index);
      },
      [](first_order) { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  static
  void readable(
    tribool result, solver &solver, formula f, std::optional<formula> muc
  )
  {
    if(result == tribool::undef) {
      io::println("UNKNOWN (stopped at k = {})", solver.last_bound());
      return;
    }

    if(result == false) {
      io::println("UNSAT");
      if(cli::unsat_core) {
        black_assert(muc.has_value());
        io::println("MUC: {}", *muc);

        if(cli::debug == "uc-replacements") {
          io::println("Replacements:");
          size_t last_index = 0;
          print_uc_replacements(*muc, last_index);
        }
        
      }
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
  void json(
    tribool result, solver &solver, formula f, std::optional<formula> muc
  ) {
    io::println("{{");
    
    io::println("    \"result\": \"{}\",", 
      result == tribool::undef ? "UNKNOWN" : // LCOV_EXCL_LINE
      result == true  ? "SAT" : "UNSAT"
    );

    io::println("    \"k\": {}{}", 
      solver.last_bound(),
      (cli::print_model && result == true) || 
      (cli::unsat_core && result == false) ? "," : ""
    );

    if(result == false && cli::unsat_core) {
      black_assert(muc.has_value());
      io::println("    \"muc\": \"{}\"", to_string(*muc));
    }

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

  void output(
    tribool result, solver &solver, formula f, std::optional<formula> muc
  ) {
    if(cli::output_format == "json")
      return json(result, solver, f, muc);

    return readable(result, solver, f, muc);
  }
  
  void trace(black::solver::trace_t data) {
    auto [type, v] = data; // LCOV_EXCL_LINE

    if(type == black::solver::trace_t::nnf && cli::debug == "print") {
      io::println(
        "{}: debug: parsed formula in NNF: {}",
        cli::command_name, to_string(std::get<formula>(v))
      );
    }

    static size_t k = 0;
    if(type == black::solver::trace_t::stage && 
       (cli::debug == "trace" || cli::debug == "trace-full")
    ) {
      k = std::get<size_t>(v);
      io::errorln("- k: {}", k);
    }

    if(cli::debug != "trace-full")
      return;

    switch(type){
      case black::solver::trace_t::stage:
      case black::solver::trace_t::nnf:
        break;
      case black::solver::trace_t::unrav:
        io::errorln("  - {}-unrav: {}", k, to_string(std::get<formula>(v)));
        break;
      case black::solver::trace_t::empty:
        io::errorln("  - {}-empty: {}", k, to_string(std::get<formula>(v)));
        break;
      case black::solver::trace_t::loop:
        io::errorln("  - {}-loop: {}", k, to_string(std::get<formula>(v)));
        break;
      case black::solver::trace_t::prune:
        io::errorln("  - {}-prune: {}", k, to_string(std::get<formula>(v)));
        break;
    }
  }

}
