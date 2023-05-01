//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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
//

#ifndef BLACK_AUTOMATON_HPP
#define BLACK_AUTOMATON_HPP

#include <black/bdd/bdd.hpp>

namespace black_internal {

  struct automaton {
    black::bdd::manager *manager;

    std::vector<black::proposition> letters;
    std::vector<black::proposition> variables;

    black::bdd::node init;
    black::bdd::node trans;
    black::bdd::node finals;
  };

  // struct LTLXFG : logic::make_combined_fragment_t<
  //   logic::propositional,
  //   logic::make_fragment_t<
  //     logic::syntax_list<
  //       logic::syntax_element::eventually,
  //       logic::syntax_element::always,
  //       logic::syntax_element::tomorrow,
  //       logic::syntax_element::w_tomorrow
  //     >
  //   >
  // > { };

  automaton to_automaton(black::bdd::manager *, logic::formula<logic::LTLP>);
  automaton to_automaton_incremental(
    black::bdd::manager *, logic::formula<logic::LTL>
  );

  automaton semideterminize(automaton);

  template<black::fragment S>
  inline automaton print(logic::formula<S> f, automaton aut) {
    std::cerr << "\nautomaton for: " << black::to_string(f) << "\n";
    std::cerr << " - letters:\n";
    for(auto v : aut.letters)
      std::cerr << "   - " << black::to_string(v) << "\n";
    std::cerr << " - declared vars:\n";
    for(auto v : aut.variables)
      std::cerr << "   - " << black::to_string(v) << "\n";
    std::cerr << " - used vars:\n";
    std::cerr << "   - init:\n";
    for(auto v : aut.init.variables())
      std::cerr << "     - " << black::to_string(v.name()) << "\n";
    std::cerr << "   - trans:\n";
    for(auto v : aut.trans.variables())
      std::cerr << "     - " << black::to_string(v.name()) << "\n";
    std::cerr << "   - finals:\n";
    for(auto v : aut.finals.variables())
      std::cerr << "     - " << black::to_string(v.name()) << "\n";

    return aut;
  }

}

#endif // BLACK_AUTOMATON_HPP
