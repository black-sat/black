//
// Created by gabriele on 16/07/2020.
//

#ifndef BLACK_TRANSLATOR_HPP
#define BLACK_TRANSLATOR_HPP

#include <black/logic/alphabet.hpp>

namespace black::internal {

  formula substitute_past(alphabet &, formula);

  std::vector<formula> gen_semantics(alphabet &, formula);

  formula conjoin_list(std::vector<formula> fs);

  formula ltlpast_to_ltl(alphabet &, formula);

  // Return ( LTL formula (without past), Set with semantics )
//  std::pair<formula, std::vector<formula>> remove_past(alphabet &, formula);
} // end namespace black::internal

// Names exported to the user
namespace black {
  using internal::ltlpast_to_ltl;
}

#endif //BLACK_TRANSLATOR_HPP
