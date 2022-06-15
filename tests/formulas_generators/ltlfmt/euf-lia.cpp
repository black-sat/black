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

#include <black/logic//alphabet.hpp>
#include <black/logic//formula.hpp>
#include <black/logic//parser.hpp>
#include <black/logic//prettyprint.hpp>

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
void generate_category_2(alphabet &sigma, int64_t n);

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

// Generate benchmarks for EUF-LIA theory and category 1
void generate_category_1 (alphabet &sigma, int64_t l) {
  variable n    = sigma.var("n");
  variable c    = sigma.var("c");
  constant zero = sigma.constant(0);
  function f("f");

  // n = 0 & c >= 0
  formula basecase = n == zero;
  basecase = basecase && (c >= zero);

  // G(wnext(c) = c & wnext(n) = n+1)
  formula constantness = G( wnext(c) == c && wnext(n) == n+1);

  // n > 1 -> f(n) = 2 f(n-1) + c
  formula body = implies(n > 1 , f(n) == 2 * f(n-1) + c);
  // n = 1 -> f(n) = c
  body = body && implies(n == 1 , f(n) == c);
  // G(body)
  body = G(body);

  // X^N(wX false) 
  formula length = wX(sigma.bottom());
  for(int64_t i=0; i<l; i++){
    length = X(length);
  }

  std::cout << to_string(basecase && constantness && body && length) << "\n"; 
}

// Generate benchmarks for EUF-LIA theory and category 2
void generate_category_2 (alphabet &sigma, int64_t l) {
  variable i    = sigma.var("i");
  variable m    = sigma.var("m");
  variable n    = sigma.var("n");
  variable s    = sigma.var("s");
  constant zero = sigma.constant(0);
  constant one  = sigma.constant(1);
  function f("f");

  // i = n = m = 0
  formula basecase = (s == one && i==zero && m == zero && n == zero);

  formula enumeration = (
    implies(
      s > 0 && n < i, 
      wnext(i) == i && wnext(n) == n + 1 && wnext(m) == m - 1 && wnext(s) == s
    ) && implies(
      s > 0 && n == i,
      wnext(i) == i + 1 && wnext(n) == i + 1 && wnext(m) == 0 && wnext(s) == 0-s
    ) && implies(
      s < 0 && m < i,
      wnext(i) == i && wnext(m) == m + 1 && wnext(n) == n - 1 && wnext(s) == s
    ) && implies(
      s < 0 && m == i,
      wnext(i) == i + 1 && wnext(m) == i + 1 && wnext(n) == 0 && wnext(s) == 0-s
    )
  );
  enumeration = G(enumeration);

  // f(0,n)=n+1 & f(m+1,0)=f(m,1) & f(m+1,n+1)=f(m,f(m+1,n))
  formula ackermann = (
    f(zero,n)   == n+one &&
    f(m+one,zero) == f(m,one) &&
    f(m+one,n+one)  == f(m,f(m+one,n))
  );

  ackermann = G(ackermann);

  // X^n( f(m,n) == 42 )
  formula length = f(m,n) == sigma.constant(42);
  for(int64_t c=0; c<l; c++){
    length = X(length);
  };

  std::cout << to_string(basecase && enumeration && ackermann && length) << "\n"; 
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
    case CATEGORY2:
      generate_category_2(sigma, n);
      break;
    default:
      print_error_and_help(argv[0], fmt::format("unknown category {}", category));
  }


  return 0;
}
