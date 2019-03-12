//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#include <iostream>
#include <variant>

#include <fmt/format.h>

#include <black/support/common.hpp>
#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>

using namespace black;

std::string print_formula(formula f);

void prova(std::variant<until,since> v);

int main()
{
  alphabet sigma;

  atom p = sigma.var("p"), q = sigma.var("q");

  formula f = (not p and q) or not q;

  fmt::print("Formula: {}\n", print_formula(f));

  fmt::print("Changing the world, one solver at the time...\n");

  return 0;
}

std::string print_formula(formula f) {
  return f.match(
    [](atom a)       { return fmt::format("{}", a.name()); },
    [](negation n)   { return fmt::format("!{}", print_formula(n.lhs())); },
    [](conjunction c){
      return fmt::format("({} and {})",
                         print_formula(c.lhs()), print_formula(c.rhs()));
    },
    [](disjunction c){
      return fmt::format("({} or {})",
                         print_formula(c.lhs()), print_formula(c.rhs()));
    },
    [](auto) {
      return fmt::format("<other>");
    }
  );
}
