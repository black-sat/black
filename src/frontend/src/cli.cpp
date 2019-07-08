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

#include <clipp.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>

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
  template<typename Cli>
  static void print_help(Cli cli) {
    auto fmt = clipp::doc_formatting{}
         .first_column(3)
         .doc_column(25)
         .last_column(79);

    fmt::print("\nBLACK - Bounded Lᴛʟ sAtisfiability ChecKer\n\n");

    fmt::print("{}\n", clipp::make_man_page(cli, cli::command_name, fmt));
  }

  //
  // main command-line parsing entry-point
  //
  void parse_command_line(int argc, char **argv)
  {
    using namespace clipp;

    bool help = false;

    auto cli = (
      (option("-k", "--bound") & integer("bound", cli::bound))
        % "maximum bound for BMC procedures",
      value("file", cli::filename).required(false)
        % "input formula file name.\n"
          "If missing, runs in interactive mode.\n"
          "If '-', reads from standard input in batch mode."
    ) | option("-h", "--help").set(help) % "print this help message";

    cli::command_name = argv[0];

    bool result = (bool)parse(argc, argv, cli);
    if(!result) {
      fmt::print(std::cerr,
        "{0}: invalid command line arguments.\n"
        "{0}: Type `{0} --help` for help.\n", argv[0]);

      std::exit(status_codes::command_line_error);
    }

    if(help) {
      print_help(cli);
      std::exit(status_codes::success);
    }
  }
}
