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

#include <black/logic/alphabet.hpp>
#include <black/logic/parser.hpp>
#include <black/solver/solver.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <fstream>
#include <iostream>
#include <string>
using namespace std::literals;

using namespace black;

inline void report_error(std::string const&s) {
  std::cerr << fmt::format("Error parsing formula: {}\n", s);
}

int batch(std::string filename);

int main(int argc, char **argv)
{
  if(argc >= 3 && argv[1] == "-f"s)
    return batch(argv[2]);

  alphabet sigma;
  solver slv(sigma);

  while(!std::cin.eof()) {
    std::string line;

    fmt::print("Please enter formula: ");
    std::getline(std::cin, line);

    std::optional<formula> f = parse_formula(sigma, line, report_error);

    if(!f)
      continue;

    fmt::print("Parsed formula: {}\n", *f);

    slv.add_formula(*f);
    bool res = slv.bsc();

    if(res)
      fmt::print("Hoooraay!\n");
    else
      fmt::print("Boooooh!\n");

    slv.clear();
  }

  return 0;
}

int batch(std::string filename) {
  std::ifstream file(filename);

  if(!file.good()) {
    std::cerr << fmt::format("Unable to open file: {}\n", filename);
    return 1;
  }

  std::string line;
  std::getline(file, line);

  alphabet sigma;

  std::optional<formula> f = parse_formula(sigma, line, report_error);

  if(!f)
    return 1;

  solver slv(sigma);

  slv.add_formula(*f);

  return int{! slv.bsc()};
}
