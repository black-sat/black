//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#ifndef BLACK_DEBUG_RANDOM_FORMULA_HPP
#define BLACK_DEBUG_RANDOM_FORMULA_HPP

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>
#include <random>

namespace black_internal::random {

  using namespace logic;

  BLACK_EXPORT
  formula random_ltl_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols);

  BLACK_EXPORT
  formula random_ltlp_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols);
  
  BLACK_EXPORT
  formula random_sltl_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols);

  BLACK_EXPORT
  formula random_boolean_formula(
      std::mt19937& gen, alphabet& sigma, int n,
      std::vector<std::string> const& symbols);

}

namespace black {
  using black_internal::random::random_ltl_formula;
  using black_internal::random::random_ltlp_formula;
  using black_internal::random::random_sltl_formula;
  using black_internal::random::random_boolean_formula;
}

#endif // BLACK_DEBUG_RANDOM_FORMULA_HPP
