//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Gabriele Venturato
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

#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/internal/debug/random_formula.hpp>

#include <iostream>
#include <functional>
#include <random>
#include <variant>
#include <cstring>

using namespace black;

enum class logic_t { ltl, ltlp, sltl };

const int DIM_MIN = 10;
const int DIM_MAX = 100;

/* ---------------------------------- MAIN ---------------------------------- */
[[ noreturn ]] void help();
std::vector<std::string> parse_ap(std::string);

void help() {
  std::cerr
      << "\nGenerator for random LTL(+Past) formulas.\n"
      << "\nUsage: random_formulas_generator [options]\n"
      << "\nOptions:\n"
      << "\t--num <x>   : Number of formulas wanted, must be x>0. Default is 1."
      << "\n"
      << "\t--dim <x>   : Dimension of each generated formula, must be x>0.\n"
      << "\t            : Default is a random number between " << DIM_MIN
      << " and " << DIM_MAX << ".\n"
      << "\t--ap <xs>   : Set of propositional given in the form '[p,q,..]'.\n"
      << "\t            : Default is a set '[p1,..,pn]' with n the log2 of the"
      << "\n"
      << "\t            : dimension of the generated formula.\n"
      << "\t--ltl       : Use only LTL operators. Default is to use LTL+Past.\n"
      << "\t--seed <x>  : Integer number used to init the random generator.\n";
  exit(1);
}

std::vector<std::string> parse_ap(std::string str_ap) {
  std::vector<std::string> ap;
  std::string p;

  if (str_ap.front() != '[' || str_ap.back() != ']') {
    std::cerr << "Malformed propositional set expression: " << str_ap << "\n";
    exit(1);
  }

  unsigned long i=1;
  while (i < str_ap.size()-1) {
    if (str_ap[i] == ',') {
      ap.push_back(p);
      p.clear();
    } else {
      if (str_ap[i] != ' ') p.push_back(str_ap[i]);
    }
    ++i;
  }

  if (!p.empty()) ap.push_back(p);

  return ap;
}

/* Produce the set of random generated LTL(+Past) formulas.
 */
int main(int argc, char **argv) {
  std::mt19937 gen((std::random_device())());
  std::uniform_int_distribution<> distrib(DIM_MIN, DIM_MAX);
  unsigned int s = 0;
  int num = 1, dim = distrib(gen);
  logic_t logic = logic_t::ltlp;
  std::vector<std::string> ap;
  bool ap_set = false;

  // Parsing CLI arguments
  int i=1;
  while (i < argc) {
    if (std::strcmp(argv[i], "--num") == 0) {
      try {
        num = std::stoi(argv[++i]);
      } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << "\n";
        exit(1);
      }
    } else if (std::strcmp(argv[i], "--seed") == 0) {
      try {
        s = (uint32_t) std::stoul(argv[++i]);
      } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << "\n";
        exit(1);
      }
      gen.seed(s);
      dim = distrib(gen); // need to re-generate the dimension
    } else if (std::strcmp(argv[i], "--dim") == 0) {
      try {
        dim = std::stoi(argv[++i]);
      } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << "\n";
        exit(1);
      }
    } else if (std::strcmp(argv[i], "--ap") == 0) {
      ap = parse_ap(argv[++i]);
      ap_set = true;
    } else if (std::strcmp(argv[i], "--ltl") == 0) {
      logic = logic_t::ltl;
    } else if (std::strcmp(argv[i], "--sltl") == 0) {
      logic = logic_t::sltl;
    } else {
      std::cerr << "Unknown argument: " << argv[i] << "\n";
      help();
    }
    ++i;
  }

  if (num < 1 || dim < 1) {
    std::cerr << "Non positive arguments are not allowed.\n";
    help();
  }

  // Generate AP if needed
  if (!ap_set && ap.empty()) {
    for (int j=1; j<=(int)floor(log2(dim)); j++) {
      ap.push_back("p" + std::to_string(j));
    }
  }

  alphabet sigma;

  // Generate <num> random formulas
  for (int j=0; j<num; j++) {
    std::optional<formula> f;
    switch(logic) {
      case logic_t::ltl:
        f = black::random_ltl_formula(gen, sigma, dim, ap);
        break;
      case logic_t::ltlp:
        f = black::random_ltlp_formula(gen, sigma, dim, ap);
        break;
      case logic_t::sltl:
        f = black::random_sltl_formula(gen, sigma, dim, ap);
        break;
    }
    std::cout << to_string(*f) << "\n";  
  }

  return 0;
}
