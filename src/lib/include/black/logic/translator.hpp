//
// Created by gabriele on 16/07/2020.
//

#ifndef BLACK_TRANSLATOR_HPP
#define BLACK_TRANSLATOR_HPP

//#include <black/logic/formula.hpp>
#include <black/logic/alphabet.hpp>

namespace black::internal {
  /*//
  // Class to translate LTL+Past into LTL formulas
  //
  class translator
  {
  public:
    // Class constructor
    translator(alphabet &a, formula f) : _alpha(a), _frm(f) { }

    formula translate();

  private:
    // Reference to the original _alphabet
    alphabet& _alpha;

    // Current LTL formula to translate
    formula _frm;

    formula substitute_past(formula);

    std::vector<formula> gen_semantics(formula);
  }; // end class translator*/

  formula remove_past(alphabet&, formula);
} // end namespace black::internal

// Names exported to the user
namespace black {
//  using internal::translator;
  using internal::remove_past;
}

#endif //BLACK_TRANSLATOR_HPP
