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

#include <black/sat/backends/minisat.hpp>
#include <black/sat/cnf.hpp>

#include <minisat/simp/SimpSolver.h>
#include <tsl/hopscotch_map.h>

BLACK_REGISTER_SAT_BACKEND(minisat)

namespace black::sat::backends
{

  // TODO: generalize with the same function in cnf.cpp
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    return f.alphabet()->var(f);
  }

  struct minisat::_minisat_t {
    Minisat::SimpSolver solver;
    cnf clauses;
  };

  minisat::minisat() : _data{std::make_unique<_minisat_t>()} { 
    _data->solver.verbosity = -1;
    _data->solver.use_elim = false;
    _data->solver.newVar();
  }

  minisat::~minisat() { }

  void minisat::assert_formula(formula f) 
  {
    cnf c = to_cnf(f);
    
    size_t new_vars = _data->clauses.add_clauses(c);
    for(size_t i = 0; i <= new_vars; ++i)
      _data->solver.newVar();
    
    for(clause cls : c.clauses()) {
      Minisat::vec<Minisat::Lit> lits;

      for(literal lit : cls.literals)
        lits.push(Minisat::mkLit(_data->clauses.var(lit.atom), lit.sign));

      _data->solver.addClause(lits);
    }
  }

  bool minisat::is_sat_with(formula assumption) {
    Minisat::vec<Minisat::Lit> lits;
    
    assert_formula(iff(fresh(assumption), assumption));
    lits.push(Minisat::mkLit(_data->clauses.var(fresh(assumption)), true));

    return _data->solver.solve(lits);
  }

  bool minisat::is_sat() {
    return _data->solver.solve();
  }

  void minisat::clear() {
    _data = std::make_unique<_minisat_t>();
  }

  std::optional<std::string> minisat::license() const
  {
    return
R"(
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
           Copyright (c) 2007-2010  Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
)";
  }
}
