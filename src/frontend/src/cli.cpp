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

#include <black/frontend/cli.hpp>
#include <black/frontend/io.hpp>
#include <black/sat/solver.hpp>
#include <black/support/config.hpp>
#include <black/support/license.hpp>

#include <clipp.h>

//
// Plug into the Clipp arguments conversion code to support std::optional vars
//
namespace clipp::detail {
  template<class T>
  struct make<std::optional<T>> {
    static inline std::optional<T> from(const char* s) {
        if(!s) return std::nullopt;
        return {make<T>::from(s)};
    }
  };
}

namespace black::frontend
{

  void quit(status_code status) {
    exit(static_cast<uint8_t>(status));
  }

  void command_line_error(std::string const& what) {
    io::error(
      "{0}: invalid command line arguments: {1}\n"
      "{0}: Type `{0} --help` for help.\n", cli::command_name, what);
  }

  static void print_header() {
    io::println("BLACK - Bounded Lᴛʟ sAtisfiability ChecKer");
    io::println("        version {}", black::version);
  }

  static void print_version() {
    static std::string sep(80, '-');

    print_header();
    io::println("{}", sep);
    io::println("{}", black::license);
    for(auto name : black::sat::solver::backends()) {
      auto backend = black::sat::solver::get_solver(name);
      if(auto l = backend->license(); l) {
        io::println("{}", sep);
        io::println("{}", *l);
      }
    }
  }

  template<typename Cli>
  static void print_help(Cli cli) {
    auto fmt = clipp::doc_formatting{}
         .first_column(3)
         .doc_column(35)
         .last_column(79);

    io::println("");
    print_header();
    io::println("");

    io::println("\n{}", clipp::make_man_page(cli, cli::command_name, fmt));
  }

  static void print_sat_backends() {
    print_header();
    io::println("\nAvailable SAT backends:");
    for(auto backend : black::sat::solver::backends()) {
      bool star = backend == BLACK_DEFAULT_BACKEND;
      io::println(" - {} {}", backend, star ? "*" : "");
    }
  }

  static bool is_backend(std::string const &name) {
    return black::sat::solver::backend_exists(name);
  }

  static bool is_output_format(std::string const &format) {
    return format == "readable" || format == "json"; // LCOV_EXCL_LINE
  }

  static bool is_sort(std::string const& s) {
    return s == "integers" || s == "reals"; // LCOV_EXCL_LINE
  }

  //
  // main command-line parsing entry-point
  //
  void parse_command_line(int argc, char **argv)
  {
    using namespace clipp;

    bool help = false;
    bool version = false;
    bool show_backends = false;

    auto cli = "solving mode: " % (
      command("solve"),
      (option("-k", "--bound") & integer("bound", cli::bound))
        % "maximum bound for BMC procedures",
      (option("-B", "--sat-backend") 
        & value(is_backend, "backend", cli::sat_backend))
        % "select the SAT backend to use",
      option("--remove-past").set(cli::remove_past)
        % "translate LTL+Past formulas into LTL before checking satisfiability",
      option("--finite").set(cli::finite)
        % "treat formulas as LTLf and look for finite models",
      option("-m", "--model").set(cli::print_model)
        % "print the model of the formula, if any",
      option("-c", "--unsat-core").set(cli::unsat_core)
        % "for unsatisfiable formulas, compute the minimum unsat core",
      (option("-d", "--domain")
        & value(is_sort, "sort", cli::domain))
        % "select the domain for first-order variables.\n"
          "Mandatory for first-order formulas.\n"
          "Accepted domains: integers, reals",
      option("-s", "--semi-decision").set(cli::semi_decision)
        % "disable termination checks for unsatisfiable formulas, speeding up "
          "the execution for satisfiable ones.\n"
          "Note: the use of `next(x)` terms in formulas implies this option.",
      (option("-o", "--output-format") 
        & value(is_output_format, "fmt", cli::output_format))
        % "Output format.\n"
          "Accepted formats: readable, json\n"
          "Default: readable",
      (option("-f", "--formula") & value("formula", cli::formula))
        % "LTL formula to solve",
      option("--debug") & value("debug", cli::debug),
      value("file", cli::filename).required(false)
          % "input formula file name.\n"
            "If '-', reads from standard input."
    ) |
    "trace checking mode: " % (
      command("check").set(cli::trace_checking), 
      (required("-t","--trace") & value("trace", cli::trace))
        % "trace file to check against the formula.\n"
          "If '-', reads from standard input.",
      (option("-e", "--expected") & value("result", cli::expected_result))
        % "expected result (useful in testing)",
      (option("-i", "--initial-state") & value("state", cli::initial_state))
        % "index of the initial state over which to evaluate the formula. "
          "Default: 0",
      option("--finite").set(cli::finite)
        % "treat formulas as LTLf and expect a finite model",
      (option("--verbose").set(cli::verbose))
        % "output a verbose log",
      (option("-f", "--formula") & value("formula", cli::formula))
        % "formula against which to check the trace",
      value("file", cli::filename).required(false)
        % "formula file against which to check the trace"
    ) | "DIMACS mode: " % (
      command("dimacs").set(cli::dimacs),
      (option("-B", "--sat-backend")
        & value(is_backend, "backend", cli::sat_backend))
        % "select the SAT backend to use",
      value("file", cli::filename)
        % "DIMACS file to solve"
    ) | command("--sat-backends").set(show_backends) 
          % "print the list of available SAT backends"
      | command("-v", "--version").set(version)
          % "show version and license information"
      | command("-h", "--help").set(help) % "print this help message";

    cli::command_name = argv[0];

    bool result = (bool)parse(argc, argv, cli);
    if(!result) {
      command_line_error("missing or unrecognized option");

      quit(status_code::command_line_error);
    }

    if(help) {
      print_help(cli);
      quit(status_code::success);
    }

    if(show_backends) {
      print_sat_backends();
      quit(status_code::success);
    }

    if(version) {
      print_version();
      quit(status_code::success);
    }
  }
}
