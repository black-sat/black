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

#include <black/sat/backends/glucose.hpp>
#include <black/sat/cnf.hpp>

#include <simp/SimpSolver.h>
#include <tsl/hopscotch_map.h>

BLACK_REGISTER_SAT_BACKEND(glucose)

namespace black::sat::backends
{

  // TODO: generalize with the same function in cnf.cpp
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    return f.alphabet()->var(f);
  }

  struct glucose::_glucose_t {
    Glucose::SimpSolver solver;
    tsl::hopscotch_map<atom, Glucose::Var> vars;

    Glucose::Var var(atom a) {
      if(auto it = vars.find(a); it != vars.end())
        return it->second;
      
      Glucose::Var v = solver.newVar();
      vars.insert({a, v});
      return v;
    }
  };

  glucose::glucose() : _data{std::make_unique<_glucose_t>()} { 
    _data->solver.verbosity = -1;
  }

  glucose::~glucose() { }

  void glucose::assert_formula(formula f) {
    cnf c = to_cnf(f);
    for(clause cls : c.clauses) {
      Glucose::vec<Glucose::Lit> lits;
      for(literal lit : cls.literals) {
        lits.push(Glucose::mkLit(_data->var(lit.atom), lit.sign));
      }
      _data->solver.addClause(lits);
    }
  }

  bool glucose::is_sat(std::vector<formula> const& assumptions) {
    Glucose::vec<Glucose::Lit> vars;
    for(formula f : assumptions) {
      assert_formula(iff(fresh(f), f));
      vars.push(Glucose::mkLit(_data->var(fresh(f)), true));
    }

    return _data->solver.solve(vars);
  }

  bool glucose::is_sat() {
    return _data->solver.solve();
  }

  void glucose::clear() {
    _data = std::make_unique<_glucose_t>();
  }
}
