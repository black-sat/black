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

#include <black/internal/debug/random_formula.hpp>

#include <black/logic/logic.hpp>

#include <variant>
#include <random>
#include <array>

namespace black_internal::random {
  
  static
  auto unary_ops(propositional) {
    std::vector<unary<propositional>::type> a = {
      unary<propositional>::type::negation{}
    };
    return a;
  }

  static
  auto unary_ops(LTL) {
    std::vector<unary<LTL>::type> a = {
      unary<LTL>::type::negation{},
      unary<LTL>::type::tomorrow{},
      unary<LTL>::type::w_tomorrow{},
      unary<LTL>::type::always{},
      unary<LTL>::type::eventually{}
    };
    return a;
  }

  static
  auto unary_ops(LTLP) {
    std::vector<unary<LTLP>::type> a = {
      unary<LTLP>::type::negation{},
      unary<LTLP>::type::tomorrow{},
      unary<LTLP>::type::w_tomorrow{},
      unary<LTLP>::type::always{},
      unary<LTLP>::type::eventually{},
      unary<LTLP>::type::yesterday{},
      unary<LTLP>::type::w_yesterday{},
      unary<LTLP>::type::once{},
      unary<LTLP>::type::historically{}
    };
    return a;
  }

  static
  auto binary_ops(propositional) {
    std::vector<binary<propositional>::type> a = {
      binary<propositional>::type::conjunction{},
      binary<propositional>::type::disjunction{},
      binary<propositional>::type::implication{},
      binary<propositional>::type::iff{}
    };
    return a;
  }

  static
  auto binary_ops(LTL) {
    std::vector<binary<LTL>::type> a = {
      binary<LTL>::type::conjunction{},
      binary<LTL>::type::disjunction{},
      binary<LTL>::type::implication{},
      binary<LTL>::type::iff{},
      binary<LTL>::type::until{},
      binary<LTL>::type::release{},
      binary<LTL>::type::w_until{},
      binary<LTL>::type::s_release{}
    };
    return a;
  }

  static
  auto binary_ops(LTLP) {
    std::vector<binary<LTLP>::type> a = {
      binary<LTLP>::type::conjunction{},
      binary<LTLP>::type::disjunction{},
      binary<LTLP>::type::implication{},
      binary<LTLP>::type::iff{},
      binary<LTLP>::type::until{},
      binary<LTLP>::type::release{},
      binary<LTLP>::type::w_until{},
      binary<LTLP>::type::s_release{},
      binary<LTLP>::type::since{},
      binary<LTLP>::type::triggered{}
    };

    return a;
  }

  template<fragment Syntax>
  class rand_formula_gen
  {
  public:
    rand_formula_gen(
        std::mt19937& gen, alphabet& sigma,
        const std::vector<std::string>& symbols
    ) : _gen{gen}, _sigma{sigma},
        _binary_ops(binary_ops(Syntax{})), _unary_ops(unary_ops(Syntax{}))
    {
      //_ap = {_sigma.top(), _sigma.bottom()};
      for (const std::string& s : symbols) {
        _ap.push_back(_sigma.proposition(s));
      }
    }

    formula<Syntax> random_formula(int n);

  private:
    std::mt19937& _gen;
    alphabet& _sigma;
    std::vector<typename binary<Syntax>::type> _binary_ops;
    std::vector<typename unary<Syntax>::type> _unary_ops;
    std::vector<formula<Syntax>> _ap;            // AP U {True,False}

    formula<Syntax> random_proposition() {
      std::uniform_int_distribution<size_t> dist(0, _ap.size()-1);
      return _ap[dist(_gen)];
    }

    typename unary<Syntax>::type random_unary_operator() {
      std::uniform_int_distribution<size_t> dist(0, _unary_ops.size()-1);
      return _unary_ops[dist(_gen)];
    }
   
    typename binary<Syntax>::type random_binary_operator() {
      std::uniform_int_distribution<size_t> dist(0, _binary_ops.size()-1);
      return _binary_ops[dist(_gen)];
    }

  }; // class rand_formula_gen

  template<fragment Syntax>
  formula<Syntax> rand_formula_gen<Syntax>::random_formula(int n) { 
    if (n == 1) {
      return random_proposition();
    } else {
      if (n == 2) {
        return unary<Syntax>(random_unary_operator(), random_formula(n-1));
      } else {
        std::uniform_int_distribution<> dist(0,1);
        bool is_unary = dist(_gen);
        
        if (is_unary) {
          return unary<Syntax>(random_unary_operator(), random_formula(n-1));
        } else {
          std::uniform_int_distribution<> bindist(1, n-2);
          int x = bindist(_gen);
          formula phi = random_formula(x);
          formula psi = random_formula(n-x-1);
          return binary<Syntax>(random_binary_operator(), phi, psi);
        }
      }
    }
  }

  formula<propositional> random_boolean_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols) {
    return rand_formula_gen<propositional>{gen, sigma, symbols}
      .random_formula(n);
  }

  formula<LTL> random_ltl_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols) {
    return rand_formula_gen<LTL>{gen, sigma, symbols}
      .random_formula(n);
  }

  formula<LTLP> random_ltlp_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols) {
    return rand_formula_gen<LTLP>{gen, sigma, symbols}
      .random_formula(n);
  }

}
