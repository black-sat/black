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
    cnf clauses;
  };

  glucose::glucose() : _data{std::make_unique<_glucose_t>()} { 
    _data->solver.verbosity = -1;
    _data->solver.newVar();
    _data->solver.use_elim = false;
  }

  glucose::~glucose() { }

  void glucose::assert_formula(formula f) {
    cnf c = to_cnf(f);
    size_t new_vars = _data->clauses.add_clauses(c);
    for(size_t i = 0; i <= new_vars; ++i)
      _data->solver.newVar();
    
    for(clause cls : c.clauses()) {
      Glucose::vec<Glucose::Lit> lits;
      for(literal lit : cls.literals) {
        lits.push(
          Glucose::mkLit(int32_t(_data->clauses.var(lit.atom)), lit.sign)
        );
      }
      _data->solver.addClause(lits);
    }
  }

  bool glucose::is_sat_with(formula assumption) {
    Glucose::vec<Glucose::Lit> lits;
    
    assert_formula(iff(fresh(assumption), assumption));
    lits.push(
      Glucose::mkLit(int32_t(_data->clauses.var(fresh(assumption))), true)
    );

    return _data->solver.solve(lits);
  }

  bool glucose::is_sat() {
    return _data->solver.solve();
  }

  void glucose::clear() {
    _data = std::make_unique<_glucose_t>();
  }

  std::optional<std::string> glucose::license() const
  {
    return 
R"(
 Glucose -- Copyright (c) 2009-2017, 
            Gilles Audemard, Laurent Simon
            CRIL - Univ. Artois, France
            LRI  - Univ. Paris Sud, France (2009-2013)
            Labri - Univ. Bordeaux, France

 Syrup (Glucose Parallel) -- Copyright (c) 2013-2014, 
                             Gilles Audemard, Laurent Simon
                             CRIL - Univ. Artois, France
                             Labri - Univ. Bordeaux, France

Glucose sources are based on MiniSat (see below MiniSat copyrights). 
Permissions and copyrights of Glucose (sources until 2013, Glucose 3.0, single 
core) are exactly the same as Minisat on which it is based on. (see below).

Glucose-Syrup sources are based on another copyright. Permissions and 
copyrights for the parallel version of Glucose-Syrup (the "Software") are 
granted, free of charge, to deal with the Software without restriction, 
including the rights to use, copy, modify, merge, publish, distribute,
sublicence, and/or sell copies of the Software, and to permit persons to whom 
the Software is furnished to do so, subject to the following conditions:

- The above and below copyrights notices and this permission notice shall be 
  included in all copies or substantial portions of the Software;
- The parallel version of Glucose (all files modified since Glucose 3.0 
  releases, 2013) cannot be used in any competitive event (sat competitions
  evaluations) without the express permission of the authors (Gilles Audemard /
  Laurent Simon). This is also the case for any competitive event using Glucose
  Parallel as an embedded SAT engine (single core or not).

--------------- Original Minisat Copyrights

Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2010, Niklas Sorensson

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
