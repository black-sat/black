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
#include <vector>


using namespace black;

void print_help(std::string const& command);
[[ noreturn ]] void print_error_and_help(std::string const& command, std::string const& error_msg);
void generate_category_1(alphabet &sigma, unsigned int n);
void generate_category_2(alphabet &sigma, unsigned int n);

// Prints the help message
void print_help(std::string const& command){
  std::cout << fmt::format("HELP: {} <category> <N> ", command) << std::endl;
  std::cout << "\tcategory : integer for the number category" << std::endl;
  std::cout << "\tN        : integer for the dimension of the benchmark" << std::endl;
}

// Prints the error message and then prints the help message
[[ noreturn ]] void print_error_and_help(std::string const& command, std::string const& error_msg)  {
  std::cerr << fmt::format("ERROR: {}.", error_msg) << std::endl;
  print_help(command);
  exit(1);
}

// Generate benchmarks for LIA theory and category 1
void generate_category_1 (alphabet &sigma, unsigned int n) {
  variable x = sigma.var("x");
  constant N = sigma.constant((int) n);

  formula f = x == 0 && G(wnext(x) == x + 1) && F(x == N);

  std::cout << to_string(f) << "\n"; 
}

// Generate benchmarks for LIA theory and category 2
void generate_category_2 (alphabet &sigma, unsigned int n) {
  // array of n variables
  std::vector<variable> variables;
  for (std::size_t i = 0; i < n; ++i) {
    variables.push_back(sigma.var("x"+std::to_string(i)));
  }
  
  // x0 > 0
  constant zero = sigma.constant(0);
  formula basecase = variables[0] > zero;

  // /\_{i=0}^{n-1} X^i ( next(x_{i+1}) > x_i )
  formula body = sigma.top();
  for(unsigned i=0; i<n; i++){
    formula next_inner = next(variables[i+1]) > variables[i];
    for(unsigned j=0; j<i; j++){
      next_inner = X(next_inner);
    }
    body = body && next_inner;
  }

  // G( /\_{i=0}^{n} wnext(x_i) = x_i )
  formula stability = sigma.top();
  for(unsigned i=0; i<=n; i++){
    stability = stability && (wnext(variables[i]) == variables[i]);
  }
  body = body && G(stability);

  // F(sum(x0 ... x_{n-1}) = n*(n+1)/2 & wX false)
  term sum = sigma.constant(0);
  for(unsigned i=0; i<=n; i++){
    sum = sum + variables[i]; 
  }
  body = body && F( sum == sigma.constant((int)(n*(n+1))/2) && wX(sigma.bottom()));

  std::cout << to_string(basecase && body) << std::endl;
}

int main(int argc, char **argv) {

  unsigned int category, n;

  if (argc != 3){
    print_error_and_help(argv[0], "wrong number of parameters");
  }

  try{
    category  = (unsigned) std::stoi(argv[1]);
    n         = (unsigned) std::stoi(argv[2]);
  } catch (const std::invalid_argument& ) {
    print_error_and_help(argv[0], "both parameters must be integers");
  }

  static alphabet sigma;
  switch (category) {
    case 1:
      generate_category_1(sigma, n);
      break;
    case 2:
      generate_category_2(sigma, n);
      break;
    default:
      print_error_and_help(argv[0], fmt::format("unknown category {}", category));
  }


  return 0;
}
