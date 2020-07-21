//
// Created by gabriele on 16/07/2020.
//

#ifndef BLACK_TRANSLATOR_HPP
#define BLACK_TRANSLATOR_HPP

#include <vector>
#include <black/logic/alphabet.hpp>

namespace black::internal {

  // Substitute past operators with new propositional letters
  formula substitute_past(alphabet &, formula);

  // Generate semantics for each new propositional letter (from substitute_past)
  std::vector<formula> gen_semantics(alphabet &, formula);

  // Put all vector elements in conjunction
  formula conjoin_list(std::vector<formula> fs);

  // Exposed procedure which puts together everything
  formula ltlpast_to_ltl(alphabet &, formula);

} // end namespace black::internal

// Names exported to the user
namespace black {
  using internal::ltlpast_to_ltl;
}

#endif //BLACK_TRANSLATOR_HPP
