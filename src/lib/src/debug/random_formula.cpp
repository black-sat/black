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
  
  class rand_formula_gen
  {
  public:
    enum fragment {
      boolean,
      LTL,
      LTLP
    };

    rand_formula_gen(
        std::mt19937& gen, alphabet& sigma,
        fragment frag,
        const std::vector<std::string>& symbols
    ) : _gen{gen}, _sigma{sigma},
        _binary_ops(binary_ops(frag)), _unary_ops(unary_ops(frag))
    {
      _ap = {_sigma.top(), _sigma.bottom()};
      for (const std::string& s : symbols) {
        _ap.push_back(_sigma.proposition(s));
      }
    }

    formula random_formula(int n);

  private:
    std::mt19937& _gen;
    alphabet& _sigma;
    std::vector<binary::type> _binary_ops;
    std::vector<unary::type> _unary_ops;
    std::vector<formula> _ap;            // AP U {True,False}

    formula random_proposition() {
      std::uniform_int_distribution<size_t> dist(0, _ap.size()-1);
      return _ap[dist(_gen)];
    }

    unary::type random_unary_operator() {
      std::uniform_int_distribution<size_t> dist(0, _unary_ops.size()-1);
      return _unary_ops[dist(_gen)];
    }
   
    binary::type random_binary_operator() {
      std::uniform_int_distribution<size_t> dist(0, _binary_ops.size()-1);
      return _binary_ops[dist(_gen)];
    }

    std::vector<unary::type> unary_ops(fragment frag) {
      switch(frag) {
        case fragment::boolean:
          return { unary::type::negation };
        case fragment::LTL:
          return {
            unary::type::negation,
            unary::type::tomorrow,
            unary::type::w_tomorrow,
            unary::type::always,
            unary::type::eventually
          };
        case fragment::LTLP:
          return {
            unary::type::negation,
            unary::type::tomorrow,
            unary::type::w_tomorrow,
            unary::type::always,
            unary::type::eventually,
            unary::type::yesterday,
            unary::type::w_yesterday,
            unary::type::once,
            unary::type::historically
          };
      }
      black_unreachable();
    }

    std::vector<binary::type> binary_ops(fragment frag) {
      switch(frag) {
        case fragment::boolean:
          return {
            binary::type::conjunction,
            binary::type::disjunction,
            binary::type::implication,
            binary::type::iff
          };
        case fragment::LTL:
          return {
            binary::type::conjunction,
            binary::type::disjunction,
            binary::type::implication,
            binary::type::iff,
            binary::type::until,
            binary::type::release,
            binary::type::w_until,
            binary::type::s_release
          };
        case fragment::LTLP:
          return {
            binary::type::conjunction,
            binary::type::disjunction,
            binary::type::implication,
            binary::type::iff,
            binary::type::until,
            binary::type::release,
            binary::type::w_until,
            binary::type::s_release,
            binary::type::since,
            binary::type::triggered
          };
      }
      black_unreachable();
    }

  }; // class rand_formula_gen

  formula rand_formula_gen::random_formula(int n) { 
    if (n == 1) {
      return random_proposition();
    } else {
      if (n == 2) {
        return unary(random_unary_operator(), random_formula(n-1));
      } else {
        std::uniform_int_distribution<> dist(0,1);
        bool is_unary = dist(_gen);
        
        if (is_unary) {
          return unary(random_unary_operator(), random_formula(n-1));
        } else {
          std::uniform_int_distribution<> bindist(1, n-2);
          int x = bindist(_gen);
          formula phi = random_formula(x);
          formula psi = random_formula(n-x-1);
          return binary(random_binary_operator(), phi, psi);
        }
      }
    }
  }

  formula random_boolean_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols) {
    return rand_formula_gen{gen, sigma, rand_formula_gen::boolean, symbols}
      .random_formula(n);
  }

  formula random_ltl_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols) {
    return rand_formula_gen{gen, sigma, rand_formula_gen::LTL, symbols}
      .random_formula(n);
  }

  formula random_ltlp_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols) {
    return rand_formula_gen{gen, sigma, rand_formula_gen::LTLP, symbols}
      .random_formula(n);
  }

}
