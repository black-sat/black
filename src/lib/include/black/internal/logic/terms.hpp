//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_LOGIC_TERMS_HPP
#define BLACK_LOGIC_TERMS_HPP

namespace black::logic {
  struct term;
  struct atom;

  namespace internal {
    struct term_custom_members {
      template<std::convertible_to<term> ...Terms>
      atom operator()(Terms ...terms) const;
    };
  }
};

namespace black::ast::core {
  template<>
  struct ast_custom_members<logic::term> 
    : logic::internal::term_custom_members { };
  
  template<ast_node_of<logic::term> Node>
  struct ast_node_custom_members<Node> 
    : logic::internal::term_custom_members { };
}

#define BLACK_AST_REFLECT_DEFS_FILE <black/internal/logic/terms-defs.hpp>
#include <black/ast/reflect>
#undef BLACK_AST_REFLECT_DEFS_FILE


namespace black::logic::internal {

  template<std::convertible_to<term> ...Terms>
  atom term_custom_members::operator()(Terms ...terms) const {
      return atom(static_cast<term const&>(*this), std::vector<term>{terms...});
  };
}

namespace black::logic {

  template<typename T, typename ...Ts>
  concept any_of = (std::same_as<T, Ts> || ...);

  template<typename T>
  concept connective = 
    any_of<T, negation, conjunction, disjunction, implication>;
  
  template<typename T>
  concept quantifier = any_of<T, exists, forall>;

  template<typename T>
  concept future =
    any_of<T, tomorrow, w_tomorrow, eventually, always, until, release>;

  template<typename T>
  concept past =
    any_of<T, yesterday, w_yesterday, once, historically, since, triggered>;

  template<typename T>
  concept temporal = future<T> || past<T>;

  template<typename T>
  concept arithmetic = any_of<T, minus, sum, product, difference, division>;
  
  template<typename T>
  concept relational = 
    any_of<T, less_than, less_than_eq, greater_than, greater_than_eq>;

}

#endif // BLACK_LOGIC_TERMS_HPP
