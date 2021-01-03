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
#include <black/logic/cnf.hpp>

#include <minisat/simp/SimpSolver.h>
#include <tsl/hopscotch_map.h>

BLACK_REGISTER_SAT_BACKEND(minisat)

namespace black::sat::backends
{

  struct minisat::_minisat_t {
    Minisat::SimpSolver solver;
    size_t nvars;
  };

  minisat::minisat() : _data{std::make_unique<_minisat_t>()} { 
    _data->solver.verbosity = -1;
    _data->solver.use_elim = false;
    _data->solver.newVar();
  }

  minisat::~minisat() { }

  void minisat::new_vars(size_t n) {
    for(size_t i = 0; i < n; ++i) {
      _data->solver.newVar();
      _data->nvars++;
    }
  }

  size_t minisat::nvars() const {
    return _data->nvars;
  }

  void minisat::assert_clause(dimacs::clause cl) { 
    Minisat::vec<Minisat::Lit> lits;
    for(dimacs::literal lit : cl.literals) {
      lits.push(Minisat::mkLit(lit.var, !lit.sign));
    }

    _data->solver.addClause(lits);
  }
  
  bool minisat::is_sat() {
    return _data->solver.solve();
  }

  bool minisat::is_sat_with(std::vector<dimacs::literal> const& assumptions) {
    Minisat::vec<Minisat::Lit> lits;
    for(dimacs::literal lit : assumptions) {
      lits.push(Minisat::mkLit(lit.var, !lit.sign));
    }

    return _data->solver.solve(lits);
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
