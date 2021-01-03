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

#include <black/sat/dimacs/solver.hpp>

namespace black::sat::dimacs::internal
{
  struct solver::_solver_t {
    tsl::hopscotch_map<atom, uint32_t> vars;

    // retrieve the var number or add it if the atom is not registered
    uint32_t var(atom a) {
      if(auto it = vars.find(a); it != vars.end())
        return it->second;

      black_assert(vars.size() < std::numeric_limits<uint32_t>::max());
      
      // the new var is size() + 1 because 0 is never assigned to any var
      uint32_t v = static_cast<uint32_t>(vars.size()) + 1;
      vars.insert({a, v});
      
      return v;
    }
  };

  solver::solver() : _data{ std::make_unique<_solver_t>() } { }

  solver::~solver() = default;

  void solver::assert_formula(formula f) 
  {
    // conversion of the formula to CNF
    cnf c = to_cnf(f);

    // census of new variables
    size_t old_size = _data->vars.size();
    for(black::clause cl : c.clauses) {
      for(black::literal lit : cl.literals) {
        _data->var(lit.atom);
      }
    }
    
    // allocate the new variables
    size_t new_size = _data->vars.size();
    if(new_size > old_size)
      this->new_vars(new_size - old_size);

    // assert the clauses
    for(black::clause cl : c.clauses) {
      dimacs::clause dcl;
      for(black::literal lit : cl.literals) {
        dcl.literals.push_back({ lit.sign, _data->var(lit.atom) });
      }

      // assert the clause
      this->assert_clause(dcl);
    }
  }

  // TODO: optimize corner cases (e.g. if assumption is already a literal)
  bool solver::is_sat_with(formula assumption) 
  { 
    atom fresh = assumption.alphabet()->var(assumption);

    this->assert_formula(iff(fresh, assumption));

    return this->is_sat_with({{true, _data->var(fresh)}});
  }

  tribool solver::value(atom a) const {
    auto it = _data->vars.find(a);
    if(it == _data->vars.end())
      return tribool::undef;

    uint32_t var = it->second;

    return this->value(var);
  }

}
