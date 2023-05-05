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

#ifndef BLACK_LOGIC_SYNTAX_HPP
#define BLACK_LOGIC_SYNTAX_HPP

#include <black/logic/core.hpp>

namespace black::logic::internal {

  // 
  // Requirements:
  // * compile-time definition of the syntax of terms
  //
  // * corresponding runtime representation 
  //
  // * rule-based system
  //
  // * a fragment is a *disjunction* of rules among the following:
  //   - set: a list of admitted types for the node and its children
  //   - head: a list of admitted types for the node
  //     children: a fragment for the children
  //
  // * available operations:
  //   - union of fragments -> the disjunction of fragments is a fragment
  //   - intersection of fragments -> see tablet
  //
  // * example: literal
  //   - set: proposition
  //   - head: negation
  //     children: 
  //     - set: proposition
  //
  // * example: CNF
  //   - head: conjunction
  //     children:
  //     - head: disjunction
  //       children: literal
  //
  // * example: NNF
  //   - set: everything except negation
  //   - head: negation
  //     children:
  //     - set: proposition
  //
  // * example: LTL
  //   - set: every boolean and future temporal operator
  //
  // * example: G(pLTL)
  //   - head: globally
  //     children:
  //     - set: every boolean and past temporal operator
  //
  // * example: SafetyLTL
  //   - intersection of:
  //     - LTL
  //     - NNF
  //     - set: boolean + X, G, R
  // 
  // * needed queries:
  //   - satisfaction of a fragment by a specific node at runtime
  //     - just a recursive descent
  //   - inclusion of a fragment by another fragment at compile-time
  //     - see tablet
  //   - given a fragment and a specific node type, get the fragment of the 
  //     children
  //
  //  

  template<typename T>
  concept new_fragment = requires {
    typename T::rules;
  };

  template<typename ...Rules>
  struct make_new_fragment_t {
    using rules = std::tuple<Rules...>;
  };

  template<typename ...Rules>
  struct make_new_fragment {
    using type = make_fragment_t<Rules...>;
  };

  template<typename List>
  struct set_rule;

  template<syntax_element ...Elements>
  struct set_rule<syntax_list<Elements...>> {
    static constexpr syntax_mask_t mask {
      static_cast<size_t>(Elements)...
    };
  };

  template<typename HeadList, typename Children>
  struct head_rule;
  
  template<syntax_element ...Elements,  typename ...Children>
  struct head_rule<syntax_list<Elements...>, std::tuple<Children...>> {
    static constexpr syntax_mask_t mask {
      static_cast<size_t>(Elements)...
    };
  };

  template<typename T>
  struct is_rule : std::false_type { };

  template<typename List>
  struct is_rule<set_rule<List>> : is_syntax_list<List> { };

  template<typename List, typename ...Rules>
  struct is_rule<head_rule<List, std::tuple<Rules...>>>
    : std::conjunction<is_syntax_list<List>, is_rule<Rules>...> { };

  template<typename T>
  inline constexpr bool is_rule_v = is_rule<T>::value;

  template<typename T>
  concept syntax_rule = is_rule_v<T>;

   // List1 - List2
  template<typename List1, typename List2>
  struct syntax_list_subtract;

  template<typename List1, typename List2>
  using syntax_list_subtract_t = 
    typename syntax_list_subtract<List1, List2>::type;

  template<typename List2>
  struct syntax_list_subtract<syntax_list<>, List2>
    : std::type_identity<syntax_list<>> { };
  
  template<syntax_element E, syntax_element ...Elements, typename List2>
  struct syntax_list_subtract<
    syntax_list<E, Elements...>, List2
  > : syntax_list_unique<
        std::conditional_t<
          syntax_list_contains_v<List2, E>,
          syntax_list_subtract_t<syntax_list<Elements...>, List2>,
          syntax_list_concat_t<
            syntax_list<E>, 
            syntax_list_subtract_t<syntax_list<Elements...>, List2>
          >
        >
      > { };
    


  template<typename Rules1, typename Rules2>
  struct rules_imply;

  template<typename Rules1, typename Rules2>
  inline constexpr bool rules_imply_v = rules_imply<Rules1, Rules2>::value;

  template<syntax_rule ...Rules1, typename Rules2>
  struct rules_imply<std::tuple<Rules1...>, Rules2>
  : std::conjunction<rules_imply<Rules1, Rules2>...> { };

  //
  // Base cases...
  //
  // head(S,C) ⊆ [head(S',C')]
  //
  template<
    typename List1, syntax_rule ...Rules1,
    typename List2, syntax_rule ...Rules2
  >
  struct rules_imply<
    head_rule<List1, std::tuple<Rules1...>>,
    std::tuple<head_rule<List2, std::tuple<Rules2...>>>
  > : std::conjunction<
        syntax_list_includes<List2, List1>,
        rules_imply<std::tuple<Rules1...>, std::tuple<Rules2...>>
      > { };
  
  //
  // set(S) ⊆ [set(S')]
  //
  template<typename List1, typename List2>
  struct rules_imply<
    set_rule<List1>,
    std::tuple<set_rule<List2>>
  > : syntax_list_includes<List2, List1> { };
  
  //
  // set(S) ⊆ [head(S',C')]
  //
  template<
    typename List1,
    typename List2, syntax_rule ...Rules2
  >
  struct rules_imply<
    set_rule<List1>,
    std::tuple<head_rule<List2, std::tuple<Rules2...>>>
  > : rules_imply<
        head_rule<List1, std::tuple<set_rule<List1>>>, 
        std::tuple<head_rule<List2, std::tuple<Rules2...>>>
      > { };
  
  //
  // head(S,C) ⊆ [set(S')]
  //
  template<
    typename List1, syntax_rule ...Rules1,
    typename List2
  >
  struct rules_imply<
    head_rule<List1, std::tuple<Rules1...>>,
    std::tuple<set_rule<List2>>
  > : rules_imply<
        head_rule<List1, std::tuple<Rules1...>>, 
        std::tuple<head_rule<List2, std::tuple<set_rule<List2>>>>
      > { };

  //
  // head(S,C) ⊆ [head(S',C'), ...]
  //
  template<
    typename List1, syntax_rule ...Rules1,
    typename List2, syntax_rule ...Rules2, 
    syntax_rule...Others
  >
  struct rules_imply<
    head_rule<List1, std::tuple<Rules1...>>, 
    std::tuple<head_rule<List2, std::tuple<Rules2...>>, Others...>
  > : std::conjunction<
        rules_imply<std::tuple<Rules1...>, std::tuple<Rules2...>>,
        rules_imply<
          std::tuple<
            head_rule<
              syntax_list_subtract_t<List1, List2>, std::tuple<Rules1...>
            >
          >,
          std::tuple<Others...>
        >
      > { };

  //
  // head(S,C) ⊆ [set(S'), ...]
  //
  template<
    typename List1, syntax_rule ...Rules1,
    typename List2,
    syntax_rule...Others
  >
  struct rules_imply<
    head_rule<List1, std::tuple<Rules1...>>, 
    std::tuple<set_rule<List2>, Others...>
  > : rules_imply<
        head_rule<List1, std::tuple<Rules1...>>,
        std::tuple<head_rule<List2, std::tuple<set_rule<List2>>>, Others...>
      > { };

  //
  // set(S) ⊆ [...]
  //
  template<typename List1, syntax_rule ...Rules>
  struct rules_imply<set_rule<List1>, std::tuple<Rules...>> 
    : rules_imply<
        head_rule<List1, std::tuple<set_rule<List1>>>, 
        std::tuple<Rules...>
      > { };

  // template<typename List2, syntax_rule Rule1, syntax_rule ...Others>
  // struct rules_imply<Rule1, std::tuple<set_rule<List2>, Others...>>
  //   : rules_imply<
  //       Rule1, std::tuple<
  //         head_rule<List2, std::tuple<set_rule<List2>>>, Others...
  //       >
  //     > { };

  // template<new_fragment Fragment, new_fragment Allowed>
  // struct is_new_subfragment_of 
  //   : rules_imply<typename Fragment::rules, typename Allowed::rules> { };

  //
  // Examples
  //
  struct Boolean : make_new_fragment_t<
    set_rule<
      syntax_list<
        syntax_element::proposition, 
        syntax_element::negation, 
        syntax_element::conjunction, 
        syntax_element::disjunction,
        syntax_element::implication,
        syntax_element::iff
      >
    >
  > { };

  struct Literal : make_new_fragment_t<
    set_rule<syntax_list<syntax_element::proposition>>,
    head_rule<
      syntax_list<syntax_element::negation>,
      std::tuple<set_rule<syntax_list<syntax_element::proposition>>>
    >
  > { };

  struct NNF : make_new_fragment_t<
    head_rule<
      syntax_list<syntax_element::negation>,
      typename Literal::rules
    >,
    set_rule<
        syntax_list<
          syntax_element::proposition, 
          syntax_element::conjunction, 
          syntax_element::disjunction,
          syntax_element::implication,
          syntax_element::iff
        >
      >
  > { };

}

#endif // BLACK_LOGIC_SYNTAX_HPP
