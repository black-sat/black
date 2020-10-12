//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante, Gabriele Venturato
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

//
// This file adapts the RandomFormula(n) procedure shown in:
// 
// Tauriainen, Heikki, and Keijo Heljanko.
// “Testing LTL Formula Translation into Büchi Automata.”
// International Journal on Software Tools for Technology Transfer 4,
// no. 1 (October 1, 2002): 57–70. https://doi.org/10.1007/s100090200070.
//

#include <black/debug/random_formula.hpp>

#include <black/logic/alphabet.hpp>

#include <variant>
#include <random>

namespace black::internal {
  
  enum class logic_t { 
    boolean,
    ltl, 
    ltlp 
  };

  class rand_formula_gen 
  {
    using op_t = std::variant<unary::type, binary::type>;

  public:
    rand_formula_gen(logic_t l, const std::vector<std::string>& symbols)
      : _gen((std::random_device())()), _logic(l) 
    {
      // base boolean operators
      _ops = {
        unary::type::negation,
        binary::type::conjunction,
        binary::type::disjunction,
        binary::type::implication,
        binary::type::iff
      };

      // LTL operators
      if(_logic != logic_t::boolean) {
        _ops.insert(_ops.end(), {
          unary::type::tomorrow,
          unary::type::always,
          unary::type::eventually,
          binary::type::until,
          binary::type::release,
        });
      }

      // LTL+Past operators
      if (_logic == logic_t::ltlp) {
        _ops.insert(_ops.end(), {
          unary::type::yesterday,
          unary::type::w_yesterday,
          unary::type::once,
          unary::type::historically,
          binary::type::since,
          binary::type::triggered
        });
      }

      // Retrieve unary operators
      for (const op_t& op : _ops) {
        if (std::holds_alternative<unary::type>(op)) {
          _unary_ops.push_back(std::get<unary::type>(op));
        }
      }

      _ap = {_sigma.top(), _sigma.bottom()};
      for (const std::string& s : symbols) {
        _ap.push_back(_sigma.var("(" + s + ")"));
      }
    }

    formula random_formula(int n);

  private:
    std::mt19937 _gen;
    logic_t _logic;
    alphabet _sigma;
    std::vector<op_t> _ops;
    std::vector<unary::type> _unary_ops; // subset of _ops
    std::vector<formula> _ap;            // AP U {True,False}

    formula random_atom();
    unary::type random_unary_operator();
    op_t random_operator();

  }; // class rand_formula_gen

  formula rand_formula_gen::random_atom() {
    std::uniform_int_distribution<size_t> dist(0, _ap.size()-1);
    return _ap[dist(_gen)];
  }

  auto rand_formula_gen::random_operator() -> op_t {
    std::uniform_int_distribution<size_t> dist(0, _ops.size()-1);
    return _ops[dist(_gen)];
  }

  unary::type rand_formula_gen::random_unary_operator() {
    std::uniform_int_distribution<size_t> dist(0, _unary_ops.size()-1);
    return _unary_ops[dist(_gen)];
  }

  formula rand_formula_gen::random_formula(int n) { // must be n >= 1
    if (n == 1) {
      return random_atom();
    } else {
      if (n == 2) {
        unary::type op = random_unary_operator();
        return unary(op, random_formula(n-1));
      } else {
        op_t op = random_operator();
        if (std::holds_alternative<unary::type>(op)) {  // if unary
          return unary(std::get<unary::type>(op), random_formula(n-1));
        } else {  // if binary
          std::uniform_int_distribution<> dist(1, n-2);
          int x = dist(_gen);
          formula phi = random_formula(x);
          formula psi = random_formula(n-x-1);
          return binary(std::get<binary::type>(op), phi, psi);
        }
      }
    }
  }
  
  formula random_boolean_formula(int n, const std::vector<std::string> &sigma) {
    return rand_formula_gen{logic_t::boolean, sigma}.random_formula(n);
  }
  
  formula random_ltl_formula(int n, const std::vector<std::string> &sigma) {
    return rand_formula_gen{logic_t::ltl, sigma}.random_formula(n);
  }
  
  formula random_ltlp_formula(int n, const std::vector<std::string> &sigma) {
    return rand_formula_gen{logic_t::ltlp, sigma}.random_formula(n);
  }

}
