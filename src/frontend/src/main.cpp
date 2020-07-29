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
#include <black/frontend/support.hpp>

#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/solver/solver.hpp>

#include <iostream>
#include <fstream>
#include <type_traits>

using namespace black::frontend;

int batch();
int batch(std::optional<std::string>, std::istream &);
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
    return batch(std::nullopt, std::cin);

  std::ifstream file{*cli::filename, std::ios::in};

  if(!file)
    io::fatal(status_code::filesystem_error,
      "Unable to open file `{}`: {}",
      *cli::filename, system_error_string(errno)
    );

  return batch(cli::filename, file);
}

int batch(std::optional<std::string> path, std::istream &file)
{
  std::string line;
  std::getline(file, line);

  black::alphabet sigma;

  std::optional<black::formula> f =
    black::parse_formula(sigma, line, [&path](auto error) {
      io::fatal(status_code::syntax_error, 
                "syntax error: {}: {}\n", 
                path ? *path : "<stdin>", error);
    });

  black_assert(f.has_value());

  black::solver slv{sigma};

  slv.assert_formula(*f);

  bool res = slv.solve(cli::bound);

  if(res)
    io::message("SAT\n");
  else
    io::message("UNSAT\n");

  return 0;
}

int interactive()
{
  black::alphabet sigma;
  black::solver slv{sigma};

  while(!std::cin.eof()) {
    std::string line;

    io::message("Please enter formula: ");
    std::getline(std::cin, line);

    std::optional<black::formula> f =
      black::parse_formula(sigma, line, [](auto error) {
        io::error("Syntax error: {}\n", error);
      });

    if(!f)
      continue;

    io::message("Parsed formula (nnf): {}\n", black::to_nnf(*f));
    if(cli::bound)
      io::message("Solving (up to k={})...\n", *cli::bound);
    else
      io::message("Solving...\n");

    slv.assert_formula(*f);
    bool res = slv.solve(cli::bound);

    if(res)
      io::message("The formula is SAT!\n\n");
    else
      io::message("The formula is UNSAT!\n\n");

    slv.clear();
  }

  return 0;
}
