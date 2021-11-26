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

#ifndef BLACK_ALPHABET_IMPL_HPP
#define BLACK_ALPHABET_IMPL_HPP

#include <black/support/meta.hpp>

namespace black::internal {

  static constexpr std::string_view logics_names[] = {
    "QF_IDL",
    "QF_LIA",
    "QF_LRA",
    "QF_NIA",
    "QF_NRA",
    "QF_RDL",
    "QF_UFIDL",
    "QF_UFLIA",
    "QF_UFLRA",
    "QF_UFNRA"
  };

  inline sort sort_of_logic(logic l) {
    switch(l) {
      case logic::QF_IDL:
      case logic::QF_LIA:
      case logic::QF_NIA:
      case logic::QF_UFIDL:
      case logic::QF_UFLIA:
        return sort::Int;
      case logic::QF_LRA:
      case logic::QF_NRA:
      case logic::QF_RDL:
      case logic::QF_UFLRA:
      case logic::QF_UFNRA:
        return sort::Real;
    }
    black_unreachable();
  }

  inline std::optional<logic> logic_from_string(std::string const&s){
    auto it = std::find(begin(logics_names), end(logics_names), s);
    if(it == std::end(logics_names))
      return {};

    return static_cast<logic>(it - std::begin(logics_names));
  }

  inline std::string_view to_string(logic l) {
    return logics_names[to_underlying(l)];
  }

  //
  // Out-of-line definitions of methods of class `alphabet`
  //
  template<typename T, REQUIRES_OUT_OF_LINE(internal::is_hashable<T>)>
  inline proposition alphabet::prop(T&& label) {
    if constexpr(std::is_constructible_v<std::string,T>) {
      return
        proposition{this, 
          allocate_proposition(identifier{std::string{FWD(label)}})
        };
    } else {
      return proposition{this, allocate_proposition(identifier{FWD(label)})};
    }
  }

  template<typename T, REQUIRES_OUT_OF_LINE(internal::is_hashable<T>)>
  inline variable alphabet::var(T&& label) { 
    if constexpr(std::is_constructible_v<std::string,T>) {
      return
        variable{this, 
          allocate_variable(identifier{std::string{FWD(label)}})
        };
    } else {
      return variable{this, allocate_variable(identifier{FWD(label)})};
    }
  }

  inline constant alphabet::constant(int c) {
    return {this, allocate_constant(c)};
  }
  inline constant alphabet::constant(double c) {
    return {this, allocate_constant(c)};
  }

  // Out-of-line implementations from the handle_base class in formula.hpp,
  // and from the term_handle_base class in term.hpp,
  // placed here to have a complete alphabet type
  template<typename H, typename F>
  std::pair<class alphabet *, atom_t *>
  handle_base<H, F>::allocate_atom(
    relation const&r, std::vector<term> const& terms
  ) {
    black_assert(!terms.empty());
    class alphabet *sigma = terms[0]._alphabet;

    std::vector<term_base *> _terms;
    for(term t : terms)
      _terms.push_back(t._term);

    atom_t *object = sigma->allocate_atom(r, _terms);

    return {sigma, object};
  }

  template<typename H, typename F>
  template<typename FType, typename Arg>
  std::pair<alphabet *, unary_t *>
  handle_base<H, F>::allocate_unary(FType type, Arg const&arg)
  {
    // The type is templated only because of circularity problems
    static_assert(std::is_same_v<FType, unary::type>);

    // Get the alphabet from the argument
    class alphabet *sigma = arg._alphabet;

    // Ask the alphabet to actually allocate the formula
    unary_t *object = sigma->allocate_unary(type, arg._formula);

    return {sigma, object};
  }

  template<typename H, typename F>
  template<typename FType, typename Arg1, typename Arg2>
  std::pair<alphabet *, binary_t *>
  handle_base<H, F>::allocate_binary(FType type,
                                     Arg1 const&arg1, Arg2 const&arg2)
  {
    // The type is templated only because of circularity problems
    static_assert(std::is_same_v<FType, binary::type>);

    // Check that both arguments come from the same alphabet
    black_assert(arg1._alphabet == arg2._alphabet);

    // Get the alphabet from the first argument (same as the second, by now)
    class alphabet *sigma = arg1._alphabet;

    // Ask the alphabet to actually allocate the formula
    binary_t *object = sigma->allocate_binary(
      type, arg1._formula, arg2._formula
    );

    return {sigma, object};
  }

  template<typename H, typename T>
  std::pair<alphabet *, application_t *>
  term_handle_base<H, T>::allocate_application(
    function const&func, std::vector<term> const&args
  ) {
    black_assert(!args.empty());

    class alphabet *sigma = args[0]._alphabet;

    std::vector<term_base *> ts;
    for(term t : args)
      ts.push_back(t._term);

    application_t *object = sigma->allocate_application(func, ts);

    return {sigma, object};
  }

  template<typename H, typename T>
  template<typename Arg>
  std::pair<alphabet *, next_t *>
  term_handle_base<H, T>::allocate_next(Arg arg) {
    class alphabet *sigma = arg._alphabet;

    next_t *object = sigma->allocate_next(arg._term);

    return {sigma, object};
  }
  
} // namespace black::internal

/*
 * Functions from formula.hpp and term.hpp that need the alphabet class
 */
namespace black::internal {
   
  //
  // Operators from term.hpp
  //
  inline application operator-(term t) {
    return application(function::negation, {t});
  }

  inline application operator-(term t1, term t2) {
    return application{function::subtraction, {t1, t2}};
  }

  inline application operator-(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return application{function::subtraction, {t1, t2}};
  }

  inline application operator-(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return application{function::subtraction, {t1, t2}};
  }
  
  inline application operator+(term t1, term t2) {
    return application{function::addition, {t1, t2}};
  }

  inline application operator+(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return application{function::addition, {t1, t2}};
  }

  inline application operator+(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return application{function::addition, {t1, t2}};
  }
  
  inline application operator*(term t1, term t2) {
    return application{function::multiplication, {t1, t2}};
  }

  inline application operator*(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return application{function::multiplication, {t1, t2}};
  }

  inline application operator*(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return application{function::multiplication, {t1, t2}};
  }
  
  inline application operator/(term t1, term t2) {
    return application{function::division, {t1, t2}};
  }

  inline application operator/(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return application{function::division, {t1, t2}};
  }

  inline application operator/(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return application{function::division, {t1, t2}};
  }

  //
  // Operators from formula.hpp
  //
  // shorthands for known relations
  inline atom operator==(term t1, term t2) {
    return atom{relation::equal, {t1, t2}};
  }
  
  inline atom operator==(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return atom{relation::equal, {t1, t2}};
  }

  inline atom operator==(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return atom{relation::equal, {t1, t2}};
  }

  inline atom operator!=(term t1, term t2) {
    return atom{relation::not_equal, {t1, t2}};
  }
  
  inline atom operator!=(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return atom{relation::not_equal, {t1, t2}};
  }

  inline atom operator!=(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return atom{relation::not_equal, {t1, t2}};
  }

  inline atom operator<(term t1, term t2) {
    return atom{relation::less_than, {t1, t2}};
  }
  
  inline atom operator<(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return atom{relation::less_than, {t1, t2}};
  }

  inline atom operator<(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return atom{relation::less_than, {t1, t2}};
  }

  inline atom operator<=(term t1, term t2) {
    return atom{relation::less_than_equal, {t1, t2}};
  }
  
  inline atom operator<=(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return atom{relation::less_than_equal, {t1, t2}};
  }

  inline atom operator<=(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return atom{relation::less_than_equal, {t1, t2}};
  }

  inline atom operator>(term t1, term t2) {
    return atom{relation::greater_than, {t1, t2}};
  }
  
  inline atom operator>(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return atom{relation::greater_than, {t1, t2}};
  }

  inline atom operator>(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return atom{relation::greater_than, {t1, t2}};
  }

  inline atom operator>=(term t1, term t2) {
    return atom{relation::greater_than_equal, {t1, t2}};
  }
  
  inline atom operator>=(term t1, int v) {
    term t2 = t1.sigma()->constant(v);
    return atom{relation::greater_than_equal, {t1, t2}};
  }

  inline atom operator>=(int v, term t2) {
    term t1 = t2.sigma()->constant(v);
    return atom{relation::greater_than_equal, {t1, t2}};
  }

  
  // Conjunct multiple formulas generated from a range,
  // avoiding useless true formulas at the beginning of the fold
  template<typename Iterator, typename EndIterator, typename F>
  formula big_and(alphabet &sigma, Iterator b, EndIterator e, F&& f) {
    formula acc = sigma.top();

    while(b != e) {
      formula elem = std::forward<F>(f)(*b++);
      if(elem == sigma.top())
        continue;
      else if(acc == sigma.top())
        acc = elem;
      else
        acc = acc && elem;
    }

    return acc;
  }

  template<typename Range, typename F>
  formula big_and(alphabet &sigma, Range r, F&& f) {
    return big_and(sigma, begin(r), end(r), std::forward<F>(f));
  }
   
  // Disjunct multiple formulas generated from a range,
  // avoiding useless true formulas at the beginning of the fold
  template<typename Iterator, typename EndIterator, typename F>
  formula big_or(alphabet &sigma, Iterator b, EndIterator e, F&& f) 
  {
    formula acc = sigma.bottom();

    while(b != e) {
      formula elem = std::forward<F>(f)(*b++);
      if(elem == sigma.bottom())
        continue;
      else if(acc == sigma.bottom())
        acc = elem;
      else
        acc = acc || elem;
    }

    return acc;
  }

  template<typename Range, typename F>
  formula big_or(alphabet &sigma, Range r, F&& f) {
    return big_or(sigma, begin(r), end(r), std::forward<F>(f));
  }

}

#endif // BLACK_ALPHABET_IMPL_HPP
