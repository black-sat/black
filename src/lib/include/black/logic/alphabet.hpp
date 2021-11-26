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
#include <black/support/meta.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/term.hpp>

#include <string>
#include <unordered_map>
#include <deque>
#include <memory>
#include <optional>

namespace black::internal {

  //
  // List of supported SMT-LIB2 logics. 
  // We support (yet) only logics that make sense in a mono-sorted setting,
  // so no arrays nor bitvectors. Quantifiers are also not supported yet.
  //
  enum class logic {
    QF_IDL,
    QF_LIA,
    QF_LRA,
    QF_NIA,
    QF_NRA,
    QF_RDL,
    QF_UFIDL,
    QF_UFLIA,
    QF_UFLRA,
    QF_UFNRA
  };

  // The only built-in sorts supported (yet)
  enum class sort {
    Int,
    Real
  };

  // function that returns the default sort associated with a logic
  inline sort sort_of_logic(logic l);

  // functions to go to/from logics and strings
  std::optional<logic> logic_from_string(std::string const&);
  std::string_view to_string(logic);

  //
  // The alphabet class is the only entry point to create formulas.
  //
  // The only way to build formulas is to request some proposition from the 
  // alphabet, and then combine them using the logical operators defined in 
  // <formula.hpp>
  //
  // The alphabet handles memory management for formulas: memory allocated for
  // formulas is alive as long as the corresponding alphabet object is alive.
  //
  class BLACK_EXPORT alphabet
  {
  public:
    alphabet();
    ~alphabet();
    alphabet(alphabet const&) = delete; // Alphabets are non-copyable
    alphabet(alphabet &&); // but movable

    alphabet &operator=(alphabet const&) = delete; // non-copy-assignable
    alphabet &operator=(alphabet &&); // but move-assignable

    // set and get the logic currently selected. 
    // The default is no logic, (logic() returns nullopt).
    void set_logic(logic l);
    std::optional<enum logic> logic() const;

    // Entry point to obtain a trivially true or trivially false boolean formula
    struct boolean boolean(bool value);

    // Shortcuts for boolean(true) and boolean(false)
    struct boolean top();
    struct boolean bottom();

    // Entry point to obtain a proposition.
    // Propositions can be labelled by a piece of data of any type T, as long as
    // T is Hashable (see the std::unordered_map documentation for reference)
    template<typename T, REQUIRES(internal::is_hashable<T>)>
    proposition prop(T&& label);

    // Enty point to obtain variables.
    // Propositions can be labelled by a piece of data of any type T, as long as
    // T is Hashable (see the std::unordered_map documentation for reference)
    template<typename T, REQUIRES(internal::is_hashable<T>)>
    variable var(T&& label);

    // Enty point to obtain constants.
    // For now only integer constants are supported
    // TODO: Generalize constants
    struct constant constant(int c);
    struct constant constant(double c);

    // Function to obtain a formula given its unique id
    formula from_id(formula_id);

    // Function to obtain a term given its unique id
    term from_id(term_id);

  private:
    struct alphabet_impl;
    std::unique_ptr<alphabet_impl> _impl;

    template<typename, typename>
    friend struct handle_base;

    template<typename, typename>
    friend struct term_handle_base;

    // formulas allocation
    proposition_t *allocate_proposition(identifier _label);
    atom_t *allocate_atom(relation const&, std::vector<term_base *> const&);
    unary_t *allocate_unary(unary::type type, formula_base* arg);
    binary_t *
    allocate_binary(binary::type type, formula_base* arg1, formula_base* arg2);
    
    // terms allocation
    variable_t *allocate_variable(identifier _label);
    constant_t *allocate_constant(int c);
    next_t *allocate_next(term_base *arg);
    application_t*allocate_application(
      function const&func, std::vector<term_base *> const&args
    );
  };

} // namespace black::internal

namespace black {
  using internal::alphabet;
  using internal::logic;
  using internal::sort;
  using internal::sort_of_logic;
  using internal::logic_from_string;
  using internal::to_string;
}

#include <black/internal/formula/alphabet.hpp>

#endif // BLACK_ALPHABET_HPP
