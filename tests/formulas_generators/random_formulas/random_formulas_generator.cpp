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

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>

#include <iostream>
#include <functional>
#include <map>
#include <random>
#include <variant>
#include <cstring>

using namespace black;

enum class logic_t { ltl, ltlp };

using un_op_t = std::function<formula(formula)>;
using bin_op_t = std::function<formula(formula, formula)>;

const int DIM_MIN = 10;
const int DIM_MAX = 100;

struct op_t {
  std::variant<un_op_t, bin_op_t> op;
  logic_t logic;
};


/* This class implements RandomFormula(n) procedure introduced in:
 *
 * Tauriainen, Heikki, and Keijo Heljanko.
 * “Testing LTL Formula Translation into Büchi Automata.”
 * International Journal on Software Tools for Technology Transfer 4,
 * no. 1 (October 1, 2002): 57–70. https://doi.org/10.1007/s100090200070.
 *
 * with the extension to the past too.
 */
class rand_formula_gen {
public:
  rand_formula_gen() : rand_formula_gen(std::vector<std::string> {}) {}

  explicit rand_formula_gen(logic_t l) : rand_formula_gen(l, {}) {}

  explicit rand_formula_gen(const std::vector<std::string>& symbols)
      : rand_formula_gen(logic_t::ltlp, symbols) {}

  rand_formula_gen(logic_t l, const std::vector<std::string>& symbols)
    : _gen((std::random_device())()), _logic(l) {

    // LTL operators
    _ops = {
        {rand_formula_gen::un_negation,     logic_t::ltl},
        {rand_formula_gen::un_tomorrow,     logic_t::ltl},
        {rand_formula_gen::un_always,       logic_t::ltl},
        {rand_formula_gen::un_eventually,   logic_t::ltl},

        {rand_formula_gen::bin_conjunction, logic_t::ltl},
        {rand_formula_gen::bin_disjunction, logic_t::ltl},
        {rand_formula_gen::bin_implication, logic_t::ltl},
        {rand_formula_gen::bin_iff,         logic_t::ltl},
        {rand_formula_gen::bin_until,       logic_t::ltl},
//        {rand_formula_gen::bin_release,     logic_t::ltl} not handled by nuXmv
    };

    // LTL+Past operators
    if (_logic == logic_t::ltlp) {
      _ops.push_back({rand_formula_gen::un_yesterday, logic_t::ltlp});
      _ops.push_back({rand_formula_gen::un_w_yesterday, logic_t::ltlp});
      _ops.push_back({rand_formula_gen::un_once, logic_t::ltlp});
      _ops.push_back({rand_formula_gen::un_historically, logic_t::ltlp});

      _ops.push_back({rand_formula_gen::bin_since, logic_t::ltlp});
      _ops.push_back({rand_formula_gen::bin_triggered, logic_t::ltlp});
    }

    // Retrieve unary operators
    for (const op_t& o : _ops) {
      if (std::holds_alternative<un_op_t>(o.op)) {
        _unary_ops.push_back(o);
      }
    }

    // Symbols in AP
    _ap = {_sigma.top(), _sigma.bottom()};
    for (std::string s : symbols) {
      _ap.push_back(_sigma.var("(" + s + ")"));
    }
  }

  inline formula random_atom();
  inline op_t random_operator();
  inline op_t random_unary_operator();

  formula random_formula(int n);

private:
  std::mt19937 _gen;
  logic_t _logic;
  alphabet _sigma;
  std::vector<op_t> _ops;
  std::vector<op_t> _unary_ops; // subset of _ops
  std::vector<formula> _ap;     // AP U {True,False}

  // unary operators
  static inline formula un_negation(formula f) { return !(f); }
  static inline formula un_tomorrow(formula f) { return X(f); }
  static inline formula un_always(formula f) { return G(f); }
  static inline formula un_eventually(formula f) { return F(f); }
  static inline formula un_yesterday(formula f) { return Y(f); }
  static inline formula un_w_yesterday(formula f) { return Z(f); }
  static inline formula un_once(formula f) { return P(f); }
  static inline formula un_historically(formula f) { return H(f); }

  // binary operators
  static inline formula bin_conjunction(formula f1, formula f2) {
    return f1 && f2;
  }
  static inline formula bin_disjunction(formula f1, formula f2) {
    return f1 || f2;
  }
  static inline formula bin_implication(formula f1, formula f2) {
    return implies(f1, f2);
  }
  static inline formula bin_iff(formula f1, formula f2) { return iff(f1, f2); }
  static inline formula bin_until(formula f1, formula f2) { return U(f1, f2); }
  static inline formula bin_release(formula f1, formula f2) {
    return R(f1, f2);
  }
  static inline formula bin_since(formula f1, formula f2) { return S(f1, f2); }
  static inline formula bin_triggered(formula f1, formula f2) {
    return T(f1, f2);
  }

  // operators application
  static inline formula un_op_apply(const un_op_t& op, formula f) {
    return op(f);
  }

  static inline formula
  bin_op_apply(const bin_op_t& op, formula f1, formula f2) {
    return op(f1, f2);
  }
}; // class rand_formula_gen

formula rand_formula_gen::random_atom() {
  std::uniform_int_distribution<> i(0, _ap.size()-1);
  return _ap[i(_gen)];
}

op_t rand_formula_gen::random_operator() {
  std::uniform_int_distribution<> i(0, _ops.size()-1);
  return _ops[i(_gen)];
}

op_t rand_formula_gen::random_unary_operator() {
  std::uniform_int_distribution<> i(0, _unary_ops.size()-1);
  return _unary_ops[i(_gen)];
}

formula rand_formula_gen::random_formula(int n) { // must be n >= 1
  if (n == 1) {
    return random_atom();
  } else {
    if (n == 2) {
      op_t o = random_unary_operator();
      return un_op_apply(std::get<un_op_t>(o.op), random_formula(n-1));
    } else {
      op_t o = random_operator();
      if (std::holds_alternative<un_op_t>(o.op)) {  // if unary
        return un_op_apply(std::get<un_op_t>(o.op), random_formula(n-1));
      } else {  // if binary
        std::uniform_int_distribution<> r(1, n-2);
        int x = r(_gen);
        formula phi = random_formula(x);
        formula psi = random_formula(n-x-1);
        return bin_op_apply(std::get<bin_op_t>(o.op), phi, psi);
      }
    }
  }
}

void help() {
  std::cerr
      << "\nGenerator for random LTL(+Past) formulas.\n"
      << "\nUsage: random_formulas [options]\n"
      << "\nOptions:\n"
      << "\t--num <x> : Number of formulas wanted, must be x>0. Default is 1.\n"
      << "\t--dim <x> : Dimension of each generated formula, must be x>0.\n"
      << "\t          : Default is a random number between " << DIM_MIN
      << " and " << DIM_MAX << ".\n"
      << "\t--ap <xs> : Set of propositional given in the form '[p,q,..]'.\n"
      << "\t          : Default is a set '[p1,..,pn]' with n the log2 of the\n"
      << "\t          : dimension of the generated formula."
      << "\t--ltl     : Use only LTL operators. Default is to use LTL+Past.\n";
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
  int num = 1, dim = distrib(gen);
  logic_t logic = logic_t::ltlp;
  std::vector<std::string> ap;
  bool ap_set = false;

  int i=1;
  while (i < argc) {
    if (std::strcmp(argv[i], "--num") == 0) {
      try {
        num = std::stoi(argv[++i]);
      } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << "\n";
        exit(1);
      }
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

  if (!ap_set && ap.empty()) {
    for (int j=1; j<=(int)floor(log2(dim)); j++) {
      ap.push_back("p" + std::to_string(j));
    }
  }

  rand_formula_gen f {logic, ap};

  for (int j=0; j<num; j++) {
    std::cout << to_string(f.random_formula(dim)) << "\n";
  }

  return 0;
}