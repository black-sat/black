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
  // Utilities to construct intersect and unite rules from tuples
  //
  template<typename Tuple>
  struct intersect_tuple;
  
  template<typename Tuple>
  using intersect_tuple_t = typename intersect_tuple<Tuple>::type;

  template<syntax_rule ...Rules>
  struct intersect_tuple<std::tuple<Rules...>> 
    : std::type_identity<intersect<Rules...>> { };
  
  template<typename Tuple>
  struct unite_tuple;
  
  template<typename Tuple>
  using unite_tuple_t = typename unite_tuple<Tuple>::type;

  template<syntax_rule ...Rules>
  struct unite_tuple<std::tuple<Rules...>> 
    : std::type_identity<unite<Rules...>> { };

  //
  // An utility to filter repeated entries from a tuple
  //
  template <typename T, typename List>
  struct tuple_unique_ { 
    using type = T;
  };

  template <typename... T1, typename T, typename... T2>
  struct tuple_unique_<std::tuple<T1...>, std::tuple<T, T2...>>
    : std::conditional_t<
        (std::is_same_v<T, T1> || ...),
        tuple_unique_<std::tuple<T1...>, std::tuple<T2...>>,
        tuple_unique_<std::tuple<T1..., T>, std::tuple<T2...>>
    > { };

  template<typename List>
  struct tuple_unique : tuple_unique_<std::tuple<>, List> { };

  template <typename List>
  using tuple_unique_t = typename tuple_unique<List>::type;

  //
  // A normalized rule does not have repeated entries
  //
  template<syntax_rule Rule>
  struct normalize : std::type_identity<Rule> { };

  template<syntax_rule ...Rules>
  struct normalize<intersect<Rules...>> 
    : std::type_identity<
        intersect_tuple_t<tuple_unique_t<std::tuple<Rules...>>>
      > { };
  
  template<syntax_rule ...Rules>
  struct normalize<unite<Rules...>> 
    : std::type_identity<
        unite_tuple_t<tuple_unique_t<std::tuple<Rules...>>>
      > { };

  //
  // Merging of node(S) rules as far as possible
  //
  template<syntax_rule Rule>
  struct simplify_fold : std::type_identity<Rule> { };
  
  template<syntax_rule Rule>
  using simplify_fold_t = typename simplify_fold<Rule>::type;

  template<typename List1, typename List2, syntax_rule ...Rules>
  struct simplify_fold<intersect<node<List1>, node<List2>, Rules...>> 
    : normalize<
        simplify_fold_t<
          intersect<node<syntax_list_intersect_t<List1, List2>>, Rules...>
        >
      > { };
  
  template<syntax_rule Rule2, syntax_rule ...Rules>
  struct simplify_fold<intersect<node<syntax_list<>>, Rule2, Rules...>> 
    : std::type_identity<node<syntax_list<>>> { };
  
  template<syntax_rule Rule1, syntax_rule ...Rules>
  struct simplify_fold<intersect<Rule1, node<syntax_list<>>, Rules...>> 
    : std::type_identity<node<syntax_list<>>> { };

  template<syntax_rule ...Rules>
  struct simplify_fold<
    intersect<node<syntax_list<>>, node<syntax_list<>>, Rules...>
  >
    : std::type_identity<node<syntax_list<>>> { };
  
  template<syntax_rule Rule2, syntax_rule ...RulesI, syntax_rule ...Rules>
  struct simplify_fold<intersect<intersect<RulesI...>, Rule2, Rules...>> 
    : normalize<simplify_fold_t<intersect<RulesI..., Rule2, Rules...>>> { };
  
  template<syntax_rule Rule1, syntax_rule ...RulesI, syntax_rule ...Rules>
  struct simplify_fold<intersect<Rule1, intersect<RulesI...>, Rules...>> 
    : normalize<simplify_fold_t<intersect<Rule1, RulesI..., Rules...>>> { };
  
  template<syntax_rule ...RulesI1, syntax_rule ...RulesI2, syntax_rule ...Rules>
  struct simplify_fold<
    intersect<intersect<RulesI1...>, intersect<RulesI2...>, Rules...>
  > 
    : normalize<simplify_fold_t<intersect<RulesI1..., RulesI2..., Rules...>>> 
      { };
  
  template<typename List1, typename List2, syntax_rule ...Rules>
  struct simplify_fold<unite<node<List1>, node<List2>, Rules...>> 
    : normalize<
        simplify_fold_t<
          unite<
            node<syntax_list_unique_t<syntax_list_concat_t<List1, List2>>>, 
            Rules...
          >
        > 
      > { };

  template<syntax_rule Rule2, syntax_rule ...Rules>
  struct simplify_fold<unite<node<syntax_list<>>, Rule2, Rules...>> 
    : normalize<simplify_fold_t<unite<Rule2, Rules...>>> { };
  
  template<syntax_rule Rule1, syntax_rule ...Rules>
  struct simplify_fold<unite<Rule1, node<syntax_list<>>, Rules...>> 
    : normalize<simplify_fold_t<unite<Rule1, Rules...>>> { };
  
  template<syntax_rule ...Rules>
  struct simplify_fold<
    unite<node<syntax_list<>>, node<syntax_list<>>, Rules...>
  > 
    : normalize<simplify_fold_t<unite<Rules...>>> { };

  template<syntax_rule Rule2, syntax_rule ...RulesI, syntax_rule ...Rules>
  struct simplify_fold<unite<unite<RulesI...>, Rule2, Rules...>> 
    : normalize<simplify_fold_t<unite<RulesI..., Rule2, Rules...>>> { };
  
  template<syntax_rule Rule1, syntax_rule ...RulesI, syntax_rule ...Rules>
  struct simplify_fold<unite<Rule1, unite<RulesI...>, Rules...>> 
    : normalize<simplify_fold_t<unite<Rule1, RulesI..., Rules...>>> { };

  template<syntax_rule ...RulesI1, syntax_rule ...RulesI2, syntax_rule ...Rules>
  struct simplify_fold<
    unite<unite<RulesI1...>, unite<RulesI2...>, Rules...>
  > 
    : normalize<simplify_fold_t<unite<RulesI1..., RulesI2..., Rules...>>> 
      { };

  template<syntax_rule ...Rules>
  struct simplify_fold<intersect<intersect<Rules...>>> 
    : normalize<simplify_fold_t<intersect<Rules...>>> { };
  
  template<syntax_rule ...Rules>
  struct simplify_fold<unite<unite<Rules...>>> 
    : normalize<simplify_fold_t<unite<Rules...>>> { };
  
  template<typename List>
  struct simplify_fold<intersect<node<List>>> 
    : std::type_identity<node<List>> { };
  
  template<typename List>
  struct simplify_fold<unite<node<List>>> 
    : std::type_identity<node<List>> { };

  
  template<syntax_rule Rule>
  struct simplify
    : std::type_identity<Rule> { };

  template<syntax_rule Rule>
  using simplify_t = typename simplify<Rule>::type;

  template<syntax_rule ...Rules>
  struct simplify<unite<Rules...>>
    : simplify_fold<unite<simplify_t<Rules>...>> { };
  
  template<syntax_rule ...Rules>
  struct simplify<intersect<Rules...>>
    : simplify_fold<intersect<simplify_t<Rules>...>> { };  


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
  

  template<typename ListL, syntax_rule ...RulesR>
  struct rule_implies_aux<
    node<ListL>, unite<RulesR...>
  > : rule_implies_aux<tree<ListL, node<ListL>>, unite<RulesR...>> { };
  
  template<typename ListL, syntax_rule ...RulesR>
  struct rule_implies_aux<
    node<ListL>, intersect<RulesR...>
  > : rule_implies_aux<tree<ListL, node<ListL>>, intersect<RulesR...>> { };

  template<syntax_rule ...RulesL, typename ListR>
  struct rule_implies_aux<
    unite<RulesL...>, node<ListR>
  > : rule_implies_aux<unite<RulesL...>, tree<ListR, node<ListR>>> { };
  
  template<syntax_rule ...RulesL, typename ListR>
  struct rule_implies_aux<
    intersect<RulesL...>, node<ListR>
  > : rule_implies_aux<intersect<RulesL...>, tree<ListR, node<ListR>>> { };

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
    using simplified = simplify_t<unite<child_t<head_t<RulesR>, RulesR>...>>;

    static constexpr bool recursive = 
      std::is_same_v<simplified, unite<RulesR...>>;

    static constexpr bool value = std::conjunction_v<
      syntax_list_includes<syntax_list_union_t<head_t<RulesR>...>, ListL>,
      std::conditional_t<
        recursive,
        std::true_type,
        rule_implies_aux<ChildrenL, simplified>
      >
    >;
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
  > { 
    using simplified = 
      simplify_t<intersect<child_t<head_t<RulesL>, RulesL>...>>;

    static constexpr bool recursive = 
      std::is_same_v<simplified, intersect<RulesL...>>;

    static constexpr bool value = std::conjunction_v<
      syntax_list_includes<ListR, syntax_list_intersect_t<head_t<RulesL>...>>,
      std::conditional_t<
        recursive,
        std::true_type,
        rule_implies_aux<simplified, ChildrenR>
      >
    >;
  };

  template<syntax_rule Rule1, syntax_rule Rule2>
  struct rule_implies 
    : rule_implies_aux<simplify_t<expand_t<Rule1>>, simplify_t<expand_t<Rule2>>>
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

  
}

#endif // BLACK_LOGIC_SYNTAX_HPP
