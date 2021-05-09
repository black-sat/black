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

#include <black/frontend/cli.hpp>
#include <black/frontend/io.hpp>
#include <black/frontend/support.hpp>
#include <black/frontend/dimacs.hpp>

#include <black/sat/dimacs.hpp>

#include <iostream>

namespace black::frontend {
  
  static
  int dimacs(std::optional<std::string> const&, std::istream &in) 
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

  int dimacs() {
    if(*cli::filename == "-")
      return dimacs(std::nullopt, std::cin);

    std::ifstream file = open_file(*cli::filename);
    return dimacs(cli::filename, file);
  }
}
