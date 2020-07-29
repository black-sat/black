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

#ifndef BLACK_ALPHABET_HPP
#define BLACK_ALPHABET_HPP

#include <black/support/common.hpp>
#include <black/logic/formula.hpp>

#include <string>
#include <unordered_map>
#include <deque>
#include <memory>

namespace black {

  namespace internal {
    struct alphabet_impl;
  }

  //
  // The alphabet class is the only entry point to create formulas.
  //
  // The only way to build formulas is to request some atom from the alphabet,
  // and then combine them using the logical operators defined in <formula.hpp>
  //
  // The alphabet handles memory management for formulas: memory allocated for
  // formulas is alive as long as the corresponding alphabet object is alive.
  //
  class alphabet
  {
  public:
    alphabet();
    alphabet(alphabet const&) = delete; // Alphabets are non-copyable
    alphabet(alphabet &&) = default; // but movable

    // Entry point to obtain a trivially true or trivially false boolean formula
    struct boolean boolean(bool value);

    // Shortcuts for boolean(true) and boolean(false)
    struct boolean top();
    struct boolean bottom();

    // Entry point to obtain an atomic formula, i.e., a proposition variable
    // Atoms can be labelled by a piece of data of any type T, as long as
    // T is Hashable (see the std::unordered_map documentation for reference)
    template<typename T, REQUIRES(internal::is_hashable<T>)>
    atom var(T&& label);

    // Function to obtain a formula given its unique id
    formula from_id(formula_id);

  private:
    std::unique_ptr<internal::alphabet_impl> _impl;

    template<typename, typename>
    friend struct internal::handle_base;
  };

} // namespace black

#include <black/internal/alphabet_impl.hpp>

#endif // BLACK_ALPHABET_HPP
