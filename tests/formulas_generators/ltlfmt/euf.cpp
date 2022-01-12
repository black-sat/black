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

enum categories {
  CATEGORY1 = 1, 
  CATEGORY2 = 2, 
  CATEGORY3 = 3
};

void print_help(std::string const& command);
[[ noreturn ]] void print_error_and_help(std::string const& command, std::string const& error_msg);
void generate_category_1(alphabet &sigma, int64_t n);

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

// Generate benchmarks for EUF theory and category 1
void generate_category_1 (alphabet &sigma, int64_t n) {
  variable x = sigma.var("x");
  variable c = sigma.var("c");

  // x = c
  formula basecase = x == c;

  // G(wnext(c) = c)
  formula c_const = G(wnext(c) == c);

  // G(f(wnext(x)) = x & wnext(x) = g(x))
  function f("f");
  function g("g");
  formula body = G(f(wnext(x)) == x && wnext(x) == g(x));

  // X^N(f(g^N+1(c)))
  term g_N = c;
  for(auto i = 0; i<n; i++){
    g_N = g(g_N);
  }
  formula objective = f(g(g_N)) != g_N;
  for(auto i = 0; i<n; i++){
    objective = X(objective);
  }

  std::cout << to_string(basecase && c_const && body && objective) << "\n"; 
}

int main(int argc, char **argv) {

  int64_t category, n;

  if (argc != 3){
    print_error_and_help(argv[0], "wrong number of parameters");
  }

  try{
    category  = std::stoll(argv[1]);
    n         = std::stoll(argv[2]);
  } catch (const std::invalid_argument& ) {
    print_error_and_help(argv[0], "both parameters must be integers");
  } catch (const std::out_of_range& ){
    print_error_and_help(argv[0], "parameter too big");
  }

  alphabet sigma;
  switch (category) {
    case CATEGORY1:
      generate_category_1(sigma, n);
      break;
    default:
      print_error_and_help(argv[0], fmt::format("unknown category {}", category));
  }


  return 0;
}
