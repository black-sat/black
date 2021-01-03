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

#include <black/sat/sat.hpp>
#include <black/sat/cnf.hpp>

#include <cstdint>

namespace black::sat::dimacs::internal
{
  struct literal {
    bool sign; // true = positive, false = negative
    uint32_t var;
  };

  struct clause {
    std::vector<literal> literals;

    clause() = default;
    clause(std::vector<literal> lits) : literals(std::move(lits)) { }
    clause(std::initializer_list<literal> elems) : literals{elems} { }
  };

  class solver : public sat::solver 
  {
  public:
    solver();

    virtual ~solver();

    // sat::solver interface
    virtual void assert_formula(formula f);
    virtual bool is_sat_with(formula assumption);
    
    // specialized DIMACS interface

    // allocate `n' new variables for the solver
    virtual void new_vars(size_t n) = 0;

    // returns the number of variables allocated for the solver
    virtual size_t nvars() const = 0;

    // assert a new clause
    virtual void assert_clause(clause c) = 0;

    // solve the instance
    virtual bool is_sat() = 0;

    // solve the instance assuming the given literals
    virtual bool is_sat_with(std::vector<literal> const& assumptions) = 0;

    // retrieve the value of an atom after is_sat() or is_sat_with() 
    virtual tribool value(atom) const = 0;

    // clears the state of the solver
    virtual void clear() = 0;

    // License note for whatever third-party software lies under the hood
    virtual std::optional<std::string> license() const = 0;

  protected:
    // returns the index of the variable corresponding to the atom
    std::optional<uint32_t> var(atom a) const;

  private:
    struct _solver_t;
    std::unique_ptr<_solver_t> _data;
  };

}

namespace black::sat::dimacs {
  using internal::literal;
  using internal::clause;
  using internal::solver;
}

#endif // BLACK_SAT_DIMACS_HPP
