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

#define RMT_ENABLED 0

#include <Remotery.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace std::literals;

using namespace black;

inline void report_error(std::string const&s) {
  std::cerr << fmt::format("Error parsing formula: {}\n", s);
}

int batch(std::string filename, int k);

[[maybe_unused]]
static Remotery *rmt;

int main(int argc, char **argv)
{
  rmtError error = rmt_CreateGlobalInstance(&rmt);

  if( RMT_ERROR_NONE != error) {
	  fmt::print("Error launching Remotery: {}\n", error);
    return -1;
  }

  if(argc >= 3 && argv[1] == "-f"s)
  {
    int k = std::numeric_limits<int>::max();
    char *filename = argv[2];

    if(argc >= 4) {
      filename = argv[3];
      try {
        k = std::stoi(argv[2]);
      }catch(...){
        fmt::print("The specified depth is not a number: `{}`\n", argv[2]);
        return -1;
      }
    }

    return batch(filename, k);
  }


  alphabet sigma;
  solver slv(sigma);

  while(!std::cin.eof()) {
    std::string line;

    fmt::print("Please enter formula: ");
    std::getline(std::cin, line);

    std::optional<formula> f = parse_formula(sigma, line, report_error);

    if(!f)
      continue;

    fmt::print("Parsed formula (nnf): {}\n", to_nnf(*f));

    slv.add_formula(*f);
    bool res = slv.bsc_prune();
    //bool res = slv.inc_bsc();
    //bool res = slv.inc_bsc_prune();
    //bool res = slv.inc_bsc_prune();

    if(res)
      fmt::print("Hoooraay!\n");
    else
      fmt::print("Boooooh!\n");

    slv.clear();
  }

  rmt_DestroyGlobalInstance(rmt);

  return 0;
}

int batch(std::string filename, int k) {
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

  //fmt::print("Parsed formula (nnf): {}\n", to_nnf(*f));

  solver slv(sigma);

  slv.add_formula(*f);

  //int res = int{! slv.bsc()};
  rmt_LogText("start solving");
  bool res = slv.bsc_prune(k);
  rmt_LogText("end");
  //bool res = slv.bsc_prune();
  //int res = int{! slv.inc_bsc_prune()};

  if(res)
    fmt::print("SAT");
  else
    fmt::print("UNSAT");

  rmt_DestroyGlobalInstance(rmt);

  return 0;
}
