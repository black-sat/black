//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Luca Geatti
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
#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>

#include <fmt/format.h>

#include <iostream>
#include <cmath>


using namespace black;

static alphabet sigma;
static void print_help();
[[ noreturn ]] static void print_error_and_help(const std::string error_msg);
static void generate_category_1(const int n);

const std::string COMMANDNAME="lia";

// Prints the help message
static void print_help(){
  std::cout << fmt::format("HELP: {} <category> <N> ", COMMANDNAME) << std::endl;
  std::cout << "\tcategory : integer for the number category" << std::endl;
  std::cout << "\tN        : integer for the dimension of the benchmark" << std::endl;
}

// Prints the error message and then prints the help message
[[ noreturn ]] static void print_error_and_help(const std::string error_msg)  {
  std::cerr << fmt::format("ERROR: {}.", error_msg) << std::endl;
  print_help();
  exit(1);
}

// Generate benchmarks for LIA theory and category 1
static void generate_category_1 (const int n) {
  variable x = sigma.var("x");
  constant N = sigma.constant(n);

  formula f = x == 0 && G(wnext(x) == x + 1) && F(x == N);

  std::cout << to_string(f) << "\n"; 
}

int main(int argc, char **argv) {

  int category, n;

  if (argc != 3){
    print_error_and_help("wrong number of parameters");
  }

  try{
    category  = std::stoi(argv[1]);
    n         = std::stoi(argv[2]);
  } catch (const std::invalid_argument& ) {
    print_error_and_help("both parameters must be integers");
  }

  switch (category) {
    case 1:
      generate_category_1(n);
      break;
    default:
      print_error_and_help(fmt::format("unknown category {}", category));
  }


  return 0;
}
