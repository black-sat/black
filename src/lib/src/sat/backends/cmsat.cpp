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
    cnf clauses;
  };

  cmsat::cmsat() : _data{std::make_unique<_cmsat_t>()} { 
    _data->solver.new_var();
  }

  cmsat::~cmsat() { }

  void cmsat::assert_formula(formula f) 
  { 
    cnf c = to_cnf(f);
    
    size_t new_vars = _data->clauses.add_clauses(c);
    if(new_vars > 0)
      _data->solver.new_vars(new_vars);
    
    for(clause cls : c.clauses()) {
      std::vector<CMSat::Lit> lits;
      
      for(literal lit : cls.literals)
        lits.push_back(CMSat::Lit{(_data->clauses.var(lit.atom)), !lit.sign});

      _data->solver.add_clause(lits);
    }
  }

  bool cmsat::is_sat(formula assumption) 
  {
    assert_formula(iff(fresh(assumption), assumption));
    std::vector<CMSat::Lit> lits = {
      CMSat::Lit{_data->clauses.var(fresh(assumption)), false}
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

  std::optional<std::string> cmsat::license() const
  {
    return
R"(
Copyright (C) 2009-2020 Authors of CryptoMiniSat, see AUTHORS file
All rights reserved.

The general priciple of the licensing is as follows. Everything that's needed 
to run/build/install/link the system is MIT licensed. This allows easy 
distribution and running of the system everywhere. Files that have no copyright 
header are also MIT licensed. Note that in case you compile with M4RI, then 
M4RI's GPL license affects the final executable and library.

Everything else that's not needed to run/build/install/link is usually GPLv2
licensed or compatible (see the copyright headers.) The only exceptions are the
following files in docs/:
* `splncs03.bat` which is under the LPPL
* `ieee.cls` which is covered by the IEEE Copyright Policy

MIT License
===================

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
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

GPL License v2
===================

This program is free software; you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free Software 
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin 
Street, Fifth Floor, Boston, MA 02110-1301, USA.
)";
  }

}
