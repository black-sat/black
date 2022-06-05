//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_LOGIC_SUGAR_HPP_
#define BLACK_LOGIC_SUGAR_HPP_

namespace black::internal::new_api {

  #define declare_unary_formula_sugar(Func, Op) \
    template<typename T, REQUIRES(T::hierarchy == hierarchy_type::formula)> \
    auto Func(T f) { \
      return Op(f); \
    }
  
  declare_unary_formula_sugar(operator!, negation)
  declare_unary_formula_sugar(X, tomorrow)
  declare_unary_formula_sugar(wX, w_tomorrow)
  declare_unary_formula_sugar(Y, yesterday)
  declare_unary_formula_sugar(Z, w_yesterday)
  declare_unary_formula_sugar(G, always)
  declare_unary_formula_sugar(F, eventually)
  declare_unary_formula_sugar(O, once)
  declare_unary_formula_sugar(H, historically)

  #undef declare_unary_formula_sugar

  #define declare_binary_formula_sugar(Func, Op) \
    template< \
      typename F1, typename F2,  \
      REQUIRES( \
        F1::hierarchy == hierarchy_type::formula && \
        F2::hierarchy == hierarchy_type::formula \
      ) \
    > \
    auto Func(F1 f1, F2 f2) { \
      return Op(f1, f2); \
    }

  declare_binary_formula_sugar(operator&&, conjunction)
  declare_binary_formula_sugar(operator||, disjunction)
  declare_binary_formula_sugar(implies, implication)
  declare_binary_formula_sugar(U, until)
  declare_binary_formula_sugar(R, release)
  declare_binary_formula_sugar(wU, w_until)
  declare_binary_formula_sugar(sR, s_release)
  declare_binary_formula_sugar(S, since)
  declare_binary_formula_sugar(T, triggered)

  #undef declare_binary_formula_sugar

  #define declare_atom_sugar(Op, Rel) \
    template< \
      typename T1, typename T2, \
      REQUIRES( \
        T1::hierarchy == hierarchy_type::term && \
        T2::hierarchy == hierarchy_type::term \
      ) \
    > \
    auto operator Op(T1 t1, T2 t2) { \
      return atom(t1.sigma()->Rel(), std::vector{t1, t2}); \
    }

  declare_atom_sugar(<, less_than)
  declare_atom_sugar(<=, less_than_equal)
  declare_atom_sugar(>, greater_than)
  declare_atom_sugar(>=, greater_than_equal)

  #undef declare_atom_sugar

  #define declare_application_sugar(Op, Func) \
    template< \
      typename T1, typename T2, \
      REQUIRES( \
        T1::hierarchy == hierarchy_type::term && \
        T2::hierarchy == hierarchy_type::term \
      ) \
    > \
    auto operator Op(T1 t1, T2 t2) { \
      return application(t1.sigma()->Func(), std::vector{t1, t2}); \
    }

  declare_application_sugar(-, subtraction)
  declare_application_sugar(+, addition)
  declare_application_sugar(*, multiplication)
  declare_application_sugar(/, division)

  #undef declare_application_sugar

  template<typename T, REQUIRES(T::hierarchy == hierarchy_type::term)>
  auto operator-(T t) {
    return application(t.sigma()->negative(), std::vector{t});
  }

}

#endif // BLACK_LOGIC_SUGAR_HPP_
