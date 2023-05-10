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
  //   - node: a list of admitted types for the node and its children
  //   - head: a list of admitted types for the node
  //     children: a fragment for the children
  //
  // * available operations:
  //   - union of fragments -> the disjunction of fragments is a fragment
  //   - intersection of fragments -> see tablet
  //
  // * example: literal
  //   - node: proposition
  //   - head: negation
  //     children: 
  //     - node: proposition
  //
  // * example: CNF
  //   - head: conjunction
  //     children:
  //     - head: disjunction
  //       children: literal
  //
  // * example: NNF
  //   - node: everything except negation
  //   - head: negation
  //     children:
  //     - node: proposition
  //
  // * example: LTL
  //   - node: every boolean and future temporal operator
  //
  // * example: G(pLTL)
  //   - head: globally
  //     children:
  //     - node: every boolean and past temporal operator
  //
  // * example: SafetyLTL
  //   - intersection of:
  //     - LTL
  //     - NNF
  //     - node: boolean + X, G, R
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

  //
  // Available rules
  //
  template<typename List>
  struct node;

  template<typename HeadList, typename ...Children>
  struct tree;

  template<typename ...Rules>
  struct intersect;
  
  template<typename ...Rules>
  struct unite;

  template<typename T>
  struct is_syntax_rule : std::false_type { };

  template<typename List>
  struct is_syntax_rule<node<List>> : is_syntax_list<List> { };

  template<typename List, typename ...Rules>
  struct is_syntax_rule<tree<List, Rules...>>
    : std::conjunction<is_syntax_list<List>, is_syntax_rule<Rules>...> { };

  template<typename ...Rules>
  struct is_syntax_rule<intersect<Rules...>>
    : std::conjunction<is_syntax_rule<Rules>...> { };

  template<typename ...Rules>
  struct is_syntax_rule<unite<Rules...>>
    : std::conjunction<is_syntax_rule<Rules>...> { };

  template<typename T>
  inline constexpr bool is_rule_v = is_syntax_rule<T>::value;

  template<typename T>
  concept syntax_rule = is_rule_v<T>;  

  //
  // Merging of node(S) rules as far as possible
  //
  template<syntax_rule Rule>
  struct simplify
    : std::type_identity<Rule> { };
  
  template<typename List1, typename List2, syntax_rule ...Rules>
  struct simplify<intersect<node<List1>, node<List2>, Rules...>> 
    : simplify<
        intersect<node<syntax_list_intersect_t<List1, List2>>, Rules...>
      > { };
  
  template<typename List>
  struct simplify<intersect<node<List>>> 
    : std::type_identity<node<List>> { };
    
  template<typename List1, typename List2, syntax_rule ...Rules>
  struct simplify<unite<node<List1>, node<List2>, Rules...>> 
    : simplify<
        unite<
          node<syntax_list_unique_t<syntax_list_concat_t<List1, List2>>>, 
          Rules...
        >
      > { };
  
  template<typename List>
  struct simplify<unite<node<List>>> 
    : std::type_identity<node<List>> { };
    
  template<syntax_rule Rule>
  using simplify_t = typename simplify<Rule>::type;


  //
  // Expansion of `node` rules
  //
  template<syntax_rule Context, syntax_rule Rule>
  struct expand_aux;
  
  template<syntax_rule Context, syntax_rule Rule>
  using expand_aux_t = typename expand_aux<Context, Rule>::type;
  
  template<syntax_rule Context, typename List>
  struct expand_aux<Context, node<List>>
    : std::type_identity<tree<List, Context>> { };

  template<syntax_rule Context, typename Head, syntax_rule ...Children>
  struct expand_aux<Context, tree<Head, Children...>> 
    : std::type_identity<tree<Head, Children...>> { };

  template<syntax_rule Context, syntax_rule ...Rules>
  struct expand_aux<Context, intersect<Rules...>>
    : std::type_identity<intersect<expand_aux_t<Context, Rules>...>> { };

  template<syntax_rule Context, syntax_rule ...Rules>
  struct expand_aux<Context, unite<Rules...>>
    : std::type_identity<unite<expand_aux_t<Context, Rules>...>> { };

  template<syntax_rule Rule>
  struct expand : expand_aux<Rule, Rule> { };
  
  template<syntax_rule Rule>
  using expand_t = typename expand<Rule>::type;

  //
  // Extraction of the admitted elements for the head of a node
  //
  template<syntax_rule Rule>
  struct head;
  
  template<syntax_rule Rule>
  using head_t = typename head<Rule>::type;

  template<typename List>
  struct head<node<List>> : std::type_identity<List> { };

  template<typename List, syntax_rule Children>
  struct head<tree<List, Children>> : std::type_identity<List> { };

  template<syntax_rule ...Rules>
  struct head<intersect<Rules...>> 
    : syntax_list_intersect<head_t<Rules>...> { };

  template<syntax_rule ...Rules>
  struct head<unite<Rules...>> : syntax_list_union<head_t<Rules>...> { };

  //
  // Extraction of the children rule of a node given a set of heads for the node
  //
  template<typename Head, syntax_rule Rule>
  struct child;
  
  template<typename Head, syntax_rule Rule>
  using child_t = typename child<Head, Rule>::type;

  template<typename Head, typename List>
  struct child<Head, node<List>>
    : std::conditional<
        syntax_list_includes_v<List, Head>,
        node<List>,
        node<syntax_list<>>
      > { };

  template<typename Head, typename List, syntax_rule Children>
  struct child<Head, tree<List, Children>>
    : std::conditional<
        syntax_list_includes_v<List, Head>,
        expand_t<Children>,
        node<syntax_list<>>
      > { };

  template<typename Head, syntax_rule ...Rules>
  struct child<Head, intersect<Rules...>>
    : std::type_identity<intersect<child_t<Head, Rules>...>> { };

  template<typename Head, syntax_rule ...Rules>
  struct child<Head, unite<Rules...>>
    : std::type_identity<unite<child_t<Head, Rules>...>> { };

  //
  // Inclusion of rules.
  //
  //
  template<syntax_rule Rule1, syntax_rule Rule2>
  struct rule_implies_aux;
  
  template<syntax_rule Rule1, syntax_rule Rule2>
  inline constexpr bool rule_implies_aux_v = 
    rule_implies_aux<Rule1, Rule2>::value;

  //
  // R1 or ... or Rn -> R
  // iff
  // R1 -> R and ... and Rn -> R
  //
  template<syntax_rule ...Rules1, syntax_rule Rule2>
  struct rule_implies_aux<unite<Rules1...>, Rule2>
    : std::conjunction<rule_implies_aux<Rules1, Rule2>...> { };
  
  //
  // R -> R1 and ... and Rn
  // iff
  // R -> R1 and ... and R -> Rn
  //
  template<syntax_rule Rule1, syntax_rule ...Rules2>
  struct rule_implies_aux<Rule1, intersect<Rules2...>>
    : std::conjunction<rule_implies_aux<Rule1, Rules2>...> { };

  //
  // node(S) -> node(S')
  // iff
  // S ⊆ S'
  //
  template<typename List1, typename List2>
  struct rule_implies_aux<node<List1>, node<List2>>
    : syntax_list_includes<List2, List1> { };

  //
  // tree(S, C) -> tree(S',C')
  // iff
  // S ⊆ S'  and C -> C'
  //
  template<
    typename ListL, syntax_rule ChildrenL, 
    typename ListR, syntax_rule ChildrenR
  >
  struct rule_implies_aux<
    tree<ListL, ChildrenL>, tree<ListR, ChildrenR>
  > : std::conjunction<
        syntax_list_includes<ListR, ListL>,
        rule_implies_aux<ChildrenL, ChildrenR>
      > { };

  //
  // node(S) -> tree(S',C')
  // iff
  // tree(S, node(S)) -> tree(S', C')
  //
  template<typename ListL, typename ListR, syntax_rule ChildrenR>
  struct rule_implies_aux<
    node<ListL>, tree<ListR, ChildrenR>
  > : rule_implies_aux<tree<ListL, node<ListL>>, tree<ListR, ChildrenR>> { };

  //
  // tree(S,C) -> node(S')
  // iff
  // tree(S,C) -> tree(S', node(S'))
  //
  template<typename ListL, syntax_rule ChildrenL, typename ListR>
  struct rule_implies_aux<
    tree<ListL, ChildrenL>, node<ListR>
  > : rule_implies_aux<tree<ListL, ChildrenL>, tree<ListR, node<ListR>>> { };

  //
  // tree(S, C) -> R1 or ... or Rn
  // iff
  // S ⊆ head(R1) ∪ .. ∪ head(Rn) and expand(C) -> child(C1) or ... or child(Cn)
  //
  template<
    typename ListL, syntax_rule ChildrenL, syntax_rule ...RulesR
  >
  struct rule_implies_aux<
    tree<ListL, ChildrenL>, unite<RulesR...>
  > { 
    static constexpr bool value = 
      syntax_list_includes_v<ListL, syntax_list_union_t<head_t<RulesR>...>> &&
      rule_implies_aux<
        expand_t<ChildrenL>, 
        simplify_t<unite<simplify_t<child_t<head_t<RulesR>, RulesR>>...>>
      >::value;
  };

  //
  // R1 and ... and Rn -> tree(S, C)
  // iff
  // head(R1) ∩ .. ∩ head(Rn) ⊆ S and 
  // child(C1) and ... and child(Cn) -> expand(C)
  //
  template<
    syntax_rule ...RulesL, 
    typename ListR, syntax_rule ChildrenR
  >
  struct rule_implies_aux<
    intersect<RulesL...>, tree<ListR, ChildrenR>
  > : std::conjunction<
        syntax_list_includes<
          syntax_list_intersect_t<head_t<RulesL>...>, ListR
        >,
        rule_implies_aux<
          intersect<child_t<head_t<RulesL>, RulesL>...>, 
          expand_t<ChildrenR>
        >
      > { };


  template<syntax_rule Rule1, syntax_rule Rule2>
  struct rule_implies 
    : rule_implies_aux<expand_t<simplify_t<Rule1>>, expand_t<simplify_t<Rule2>>>
      { };
  
  template<syntax_rule Rule1, syntax_rule Rule2>
  inline constexpr bool rule_implies_v = rule_implies<Rule1, Rule2>::value;

  
  //
  // fragments
  //
  template<typename T>
  concept new_fragment = requires {
    typename T::rule;
    requires syntax_rule<typename T::rule>;
  };

  template<syntax_rule Rule>
  struct make_new_fragment_t {
    using rule = Rule;
  };

  template<syntax_rule Rule>
  struct make_new_fragment {
    using type = make_fragment_t<Rule>;
  };

  //
  // Subfragment checking
  //
  template<new_fragment Syntax1, new_fragment Syntax2>
  struct is_new_subfragment_of 
    : rule_implies<typename Syntax1::rule, typename Syntax2::rule> { };

  template<new_fragment Syntax1, new_fragment Syntax2>
  inline constexpr bool is_new_subfragment_of_v
    = is_new_subfragment_of<Syntax1, Syntax2>::value;

  //
  // Examples
  //
  struct Boolean : make_new_fragment_t<
    node<
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
    unite<
      node<syntax_list<syntax_element::proposition>>,
      tree<
        syntax_list<syntax_element::negation>,
        node<syntax_list<syntax_element::proposition>>
      >
    >
  > { };

  struct NNF : make_new_fragment_t<
    unite<
      tree<
        syntax_list<syntax_element::negation>,
        typename Literal::rule
      >,
      node<
        syntax_list<
          syntax_element::proposition, 
          syntax_element::conjunction, 
          syntax_element::disjunction,
          syntax_element::implication,
          syntax_element::iff
        >
      >
    >
  > { };

}

#endif // BLACK_LOGIC_SYNTAX_HPP
