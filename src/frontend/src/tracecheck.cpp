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

#include <black/frontend/tracecheck.hpp>
#include <black/frontend/io.hpp>
#include <black/frontend/support.hpp>

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/support/tribool.hpp>

#include <iostream>
#include <sstream>

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace black::frontend 
{
  using trace_t = std::vector<std::map<std::string, black::tribool>>;

  static
  int check(trace_t, formula) {
    return 0;
  }

  static 
  std::vector<std::map<std::string, black::tribool>>
  parse_trace(std::optional<std::string> const&tracepath, std::istream &trace) {
    json j;
    try {
      j = json::parse(trace);
    } catch (json::parse_error& ex) {
      std::string path = tracepath ? *tracepath : "<stdin>";
      io::fatal(status_code::syntax_error, "{}:{}", *tracepath, ex.what());
    }

    return {};
  }

  static
  int trace_check(
    std::optional<std::string> const&path,
    std::istream &file,
    std::optional<std::string> const&tracepath,
    std::istream &tracefile
  ) {
    black::alphabet sigma;

    black::formula f = 
      *black::parse_formula(sigma, file, formula_syntax_error_handler(path));

    trace_t trace = parse_trace(tracepath, tracefile);

    return check(trace, f);
  }

  int trace_check() {
    if(!cli::filename && !cli::formula) {
      command_line_error("please specify a filename or the --formula option");
      quit(status_code::command_line_error);
    }

    if(*cli::filename == "-" && *cli::trace_check == "-") {
      command_line_error(
        "cannot read from stdin both the formula file and the trace file"
      );
      quit(status_code::command_line_error);
    }

    if(cli::formula) {
      std::istringstream str{*cli::formula};

      if(*cli::trace_check == "-")
        return trace_check(std::nullopt, str, std::nullopt, std::cin);

      std::ifstream tracefile = open_file(*cli::trace_check);
      return trace_check(std::nullopt, str, cli::trace_check, tracefile);
    }

    if(*cli::filename == "-") {
      std::ifstream tracefile = open_file(*cli::trace_check);
      return trace_check(std::nullopt, std::cin, cli::trace_check, tracefile);
    }

    if(*cli::trace_check == "-") {
      std::ifstream file = open_file(*cli::filename);
      return trace_check(cli::filename, file, std::nullopt, std::cin);
    }

    std::ifstream file = open_file(*cli::filename);
    std::ifstream tracefile = open_file(*cli::trace_check);
    return trace_check(cli::filename, file, cli::trace_check, tracefile);
  }
}
