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
#include <black/sat/sat.hpp>
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

  static void print_header() {
    io::message("BLACK - Bounded Lᴛʟ sAtisfiability ChecKer");
    io::message("        version {}", black::version);
  }

  static void print_version() {
    static std::string sep(80, '-');

    print_header();
    io::message("{}", sep);
    io::message("{}", black::license);
    for(auto name : black::sat::solver::backends()) {
      auto backend = black::sat::solver::get_solver(name);
      if(auto l = backend->license(); l) {
        io::message("{}", sep);
        io::message("{}", *l);
      }
    }
  }

  template<typename Cli>
  static void print_help(Cli cli) {
    auto fmt = clipp::doc_formatting{}
         .first_column(3)
         .doc_column(30)
         .last_column(79);

    io::message("");
    print_header();
    io::message("");

    io::message("\n{}", clipp::make_man_page(cli, cli::command_name, fmt));
  }

  static void print_sat_backends() {
    print_header();
    io::message("\nAvailable SAT backends:");
    for(auto backend : black::sat::solver::backends()) {
      io::message(" - {}", backend);
    }
  }

  static bool is_backend(std::string const &name) {
    return black::sat::solver::backend_exists(name);
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

    auto cli = (
      (option("-k", "--bound") & integer("bound", cli::bound))
        % "maximum bound for BMC procedures",
      (option("-B", "--sat-backend") 
        & value(is_backend, "name", cli::sat_backend))
        % "select the SAT backend to use",
      option("--remove-past").set(cli::remove_past)
        % "translate LTL+Past formulas into LTL before checking satisfiability",
      value("file", cli::filename).required(false)
        % "input formula file name.\n"
          "If missing, runs in interactive mode.\n"
          "If '-', reads from standard input in batch mode."
    ) | option("--sat-backends").set(show_backends) 
          % "print the list of available SAT backends"
      | option("-v", "--version").set(version)
          % "show version and license information"
      | option("-h", "--help").set(help) % "print this help message";

    cli::command_name = argv[0];

    bool result = (bool)parse(argc, argv, cli);
    if(!result) {
      io::error(
        "{0}: invalid command line arguments.\n"
        "{0}: Type `{0} --help` for help.\n", argv[0]);

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
