//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#ifndef BLACK_SAT_DIMACS_HPP
#define BLACK_SAT_DIMACS_HPP

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>
#include <black/sat/solver.hpp>
#include <black/logic/cnf.hpp>

#include <istream>
#include <string>
#include <cstdint>

namespace black_internal::dimacs
{
  using namespace logic;

  struct literal {
    bool sign; // true = positive, false = negative
    uint32_t var;
  };

  struct clause {
    std::vector<literal> literals;
  };

  //
  // A specialized instance of sat::solver for backends with 
  // DIMACS-based interfaces (e.g. MiniSAT and CryptoMiniSAT)
  //
  class BLACK_EXPORT solver : public black::sat::solver 
  {
  public:
    solver();
    solver(scope const&);

    virtual ~solver() override;

    // sat::solver interface
    virtual void assert_formula(logic::formula<logic::FO> f) override;
    
    virtual tribool is_sat_with(logic::formula<logic::FO> assumption) override;
    
    virtual tribool value(logic::proposition a) const override;
    
    virtual tribool value(logic::atom<logic::FO> a) const override;
    
    // specialized DIMACS interface

    // allocate `n' new variables for the solver
    virtual void new_vars(size_t n) = 0;

    // assert a new clause
    virtual void assert_clause(clause c) = 0;

    // solve the instance
    virtual tribool is_sat() override = 0;

    // solve the instance assuming the given literals
    virtual tribool is_sat_with(std::vector<literal> const& assumptions) = 0;

    // retrieve the value of a proposition after is_sat() or is_sat_with() 
    virtual tribool value(uint32_t var) const = 0;

    // clears the state of the solver
    virtual void clear() override = 0;

    // License note for whatever third-party software lies under the hood
    virtual std::optional<std::string> license() const override = 0;

  protected:
    void clear_vars();

  private:
    struct _solver_t;
    std::unique_ptr<_solver_t> _data;
  };

}

namespace black::sat::dimacs {
  using black_internal::dimacs::literal;
  using black_internal::dimacs::clause;
  using black_internal::dimacs::solver;
}

#endif // BLACK_SAT_DIMACS_HPP
