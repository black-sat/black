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

#include <black/sat/backends/cmsat.hpp>
#include <black/sat/cnf.hpp>

#include <cryptominisat5/cryptominisat.h>

#include <tsl/hopscotch_map.h>

BLACK_REGISTER_SAT_BACKEND(cmsat)

namespace black::sat::backends
{

  // TODO: generalize with the same function in cnf.cpp
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    return f.alphabet()->var(f);
  }


  struct cmsat::_cmsat_t {
    CMSat::SATSolver solver;
    tsl::hopscotch_map<atom, uint32_t> vars;
    int32_t last_var = -1;

    uint32_t var(atom a) {
      if(auto it = vars.find(a); it != vars.end())
        return it->second;
      
      uint32_t new_var = static_cast<uint32_t>(++last_var);
      vars.insert({a, new_var});
      return new_var;
    }
  };

  cmsat::cmsat() : _data{std::make_unique<_cmsat_t>()} { }

  cmsat::~cmsat() { }

  void cmsat::assert_formula(formula f) { 
    cnf c = to_cnf(f);
    for(clause cls : c.clauses) {
      std::vector<CMSat::Lit> lits;
      int32_t last_var = _data->last_var;
      
      for(literal lit : cls.literals) {
        lits.push_back(
          CMSat::Lit{static_cast<uint32_t>(_data->var(lit.atom)), !lit.sign}
        );
      }
      
      size_t new_vars = static_cast<size_t>(_data->last_var - last_var);
      if(new_vars > 0)
        _data->solver.new_vars(new_vars);
      _data->solver.add_clause(lits);
    }
  }

  bool cmsat::is_sat(formula assumption) 
  {
    assert_formula(iff(fresh(assumption), assumption));
    std::vector<CMSat::Lit> lits = {
      CMSat::Lit{_data->var(fresh(assumption)), false}
    };

    CMSat::lbool ret = _data->solver.solve(&lits);
    return ret == CMSat::l_True;
  }

  bool cmsat::is_sat() {
    CMSat::lbool ret = _data->solver.solve();
    return ret == CMSat::l_True;
  }

  void cmsat::clear() {
    _data = std::make_unique<_cmsat_t>();
  }

}
