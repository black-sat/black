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

#ifndef BLACK_CNF_HPP_
#define BLACK_CNF_HPP_

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>

#include <vector>
#include <initializer_list>
#include <memory>

namespace black_internal::cnf
{
  struct literal {
    bool sign;
    logic::proposition prop;
  };

  struct clause {
    std::vector<literal> literals;

    clause() = default;
    clause(std::vector<literal> lits) : literals(std::move(lits)) { }
    clause(std::initializer_list<literal> elems) : literals{elems} { }
  };

  struct cnf
  {
    std::vector<clause> clauses;

    cnf() = default;
    cnf(std::vector<clause> _clauses) : clauses(std::move(_clauses)) { }
    cnf(std::initializer_list<clause> elems) : clauses{elems} { }

  };

  // Removal of booleans. Internal use, exposed here for testing.
  logic::formula<logic::propositional> 
  remove_booleans(logic::formula<logic::propositional> f);

  // Tseitin conversion to CNF
  BLACK_EXPORT
  cnf to_cnf(logic::formula<logic::propositional> f);

  // Conversion of literals, clauses and cnfs to formulas
  BLACK_EXPORT
  logic::formula<logic::propositional> to_formula(literal lit);

  BLACK_EXPORT
  logic::formula<logic::propositional> to_formula(
    logic::alphabet &sigma, clause c
  );

  BLACK_EXPORT
  logic::formula<logic::propositional> to_formula(
    logic::alphabet &sigma, cnf c
  );
}

namespace black {
  using black_internal::cnf::literal;
  using black_internal::cnf::clause;
  using black_internal::cnf::cnf;
  using black_internal::cnf::to_cnf;
  using black_internal::cnf::to_formula;
}

#endif // BLACK_CNF_HPP_
