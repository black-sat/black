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
#include <black/logic/past_remover.hpp>
#include <black/solver/solver.hpp>

#include <black/sat/dimacs.hpp>

#include <iostream>
#include <fstream>
#include <type_traits>
#include <set>

using namespace black::frontend;

int ltl(std::optional<std::string>, std::istream &);
int dimacs(std::optional<std::string> path, std::istream &file);
int interactive();
void print_model(black::solver &solver);
void collect_atoms(black::formula f, std::unordered_set<black::atom> &atoms);

int main(int argc, char **argv)
{
  parse_command_line(argc, argv);

  if(!cli::filename)
    return interactive();

  if(*cli::filename == "-")
    return 
      cli::dimacs ? 
        dimacs(std::nullopt, std::cin) :
        ltl(std::nullopt, std::cin);

  std::ifstream file{*cli::filename, std::ios::in};

  if(!file)
    io::fatal(status_code::filesystem_error,
      "Unable to open file `{}`: {}",
      *cli::filename, system_error_string(errno)
    );

  return cli::dimacs ? dimacs(cli::filename, file) : ltl(cli::filename, file);
}

int ltl(std::optional<std::string> path, std::istream &file)
{
  black::alphabet sigma;

  std::optional<black::formula> f =
    black::parse_formula(sigma, file, [&path](auto error) {
      io::fatal(status_code::syntax_error, 
                "syntax error: {}: {}\n", 
                path ? *path : "<stdin>", error);
    });

  black_assert(f.has_value());

  black::solver slv{sigma};
  
  if (cli::sat_backend)
    slv.set_sat_backend(*cli::sat_backend);

  if (cli::remove_past)
    slv.assert_formula(black::remove_past(*f));
  else
    slv.assert_formula(*f);

  bool res = slv.solve(cli::bound);

  if (res) {
    io::message("SAT\n");
    if(cli::print_model)
      print_model(slv);
  }
  else
    io::message("UNSAT\n");

  return 0;
}

int dimacs(std::optional<std::string> , std::istream &in) 
{
  using namespace black::sat;
  
  std::optional<dimacs::problem> problem = 
    dimacs::parse(in, [](std::string error) {
      io::message("{}", error);
      exit(1);
    });

  if(!problem) {
    io::message("Parsing problem");
    return (int)status_code::syntax_error;
  }

  std::string backend = cli::sat_backend ? *cli::sat_backend : "z3";
  std::optional<dimacs::solution> s = dimacs::solve(*problem, backend);

  dimacs::print(std::cout, s);

  return 0;
}

int interactive()
{
  black::alphabet sigma;
  black::solver slv{sigma};

  if (cli::sat_backend)
    slv.set_sat_backend(*cli::sat_backend);

  while (!std::cin.eof()) {
    std::string line;

    io::message("Please enter formula: ");
    std::getline(std::cin, line);

    std::optional<black::formula> f =
      black::parse_formula(sigma, line, [](auto error) {
        io::error("Syntax error: {}\n", error);
      });

    if (!f)
      continue;

    black::formula f_ltl = black::remove_past(*f);

    io::message("Parsed formula: {}\n", *f);
    if (f_ltl != *f && cli::remove_past)
      io::message("Translated formula: {}\n", f_ltl);

    if (cli::bound)
      io::message("Solving (up to k={})...\n", *cli::bound);
    else
      io::message("Solving...\n");

    if (cli::remove_past)
      slv.assert_formula(f_ltl);
    else
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

void print_model(black::solver &solver) {
  black_assert(solver.model().has_value());

  io::message("Model size: {}", solver.model()->size());
  io::message("Loop at: {}", solver.model()->loop());

  std::unordered_set<black::atom> atoms;
  collect_atoms(solver.current_formula(), atoms);

  for(size_t i = 0; i < solver.model()->size(); ++i) {
    io::message("- Time step: {}", i);
    for(black::atom a : atoms) {
      black::tribool v = solver.model()->value(a, i);
      if(v == black::tribool::undef)
        io::message("  -  {} = undef", to_string(a));
      else if(v)
        io::message("  -  {} = true", to_string(a));
      else
        io::message("  -  {} = false", to_string(a));
    }
  }
  
}

void collect_atoms(black::formula f, std::unordered_set<black::atom> &atoms) {
  using namespace black;
  f.match(
    [&](atom a) {
      atoms.insert(a);
    },
    [&](unary, formula f1) {
      collect_atoms(f1, atoms);
    },
    [&](binary, formula f1, formula f2) {
      collect_atoms(f1, atoms);
      collect_atoms(f2, atoms);
    }
  );
}
