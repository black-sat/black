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
#include <black/frontend/support.hpp>

#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/solver/solver.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <fstream>
#include <type_traits>

using namespace black::frontend;

int batch();
int batch(std::istream &);
int interactive();

int main(int argc, char **argv)
{
  parse_command_line(argc, argv);

  return cli::filename ? batch() : interactive();
}

int batch()
{
  black_assert(cli::filename.has_value());

  if(*cli::filename == "-")
    return batch(std::cin);

  std::ifstream file{*cli::filename, std::ios::in};

  if(!file) {
    fmt::print(std::cerr,
      "{}: Unable to open file `{}`: {}\n",
      cli::command_name, *cli::filename, system_error_string(errno)
    );

    return status_codes::filesystem_error;
  }

  return batch(file);
}

int batch(std::istream &file)
{
  std::string line;
  std::getline(file, line);

  black::alphabet sigma;

  std::optional<black::formula> f =
    black::parse_formula(sigma, line, [](auto error) {
      fmt::print(std::cerr, "{}: Syntax error: {}\n", error);
    });

  if(!f)
    return status_codes::syntax_error;

  black::solver slv{sigma};

  slv.add_formula(*f);

  bool res = slv.inc_bsc_prune(cli::bound);

  if(res)
    fmt::print("SAT\n");
  else
    fmt::print("UNSAT\n");

  return 0;
}

int interactive()
{
  black::alphabet sigma;
  black::solver slv{sigma};

  while(!std::cin.eof()) {
    std::string line;

    fmt::print("Please enter formula: ");
    std::getline(std::cin, line);

    std::optional<black::formula> f =
      black::parse_formula(sigma, line, [](auto error) {
        fmt::print(std::cerr, "Syntax error: {}\n", error);
      });

    if(!f)
      continue;

    fmt::print("Parsed formula (nnf): {}\n", black::to_nnf(*f));
    if(cli::bound)
      fmt::print("Solving (up to k={})...\n", *cli::bound);
    else
      fmt::print("Solving...\n");

    slv.add_formula(*f);
    bool res = slv.inc_bsc_prune(cli::bound);

    if(res)
      fmt::print("The formula is SAT!\n\n");
    else
      fmt::print("The formula is UNSAT!\n\n");

    slv.clear();
  }

  return 0;
}
