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

namespace black::logic::internal 
{
  //
  // Definition of fragments
  //
  template<typename Head, typename Children>
    requires (is_syntax_list_v<Head> && is_syntax_list_v<Children>)
  struct tree_rule {
    using head = Head;
    using children = Children;
  };

  template<typename T>
  struct is_syntax_rule : std::false_type { };

  template<typename Head, typename Children>
  struct is_syntax_rule<tree_rule<Head, Children>> : std::true_type { };
  
  template<typename T>
  inline constexpr bool is_syntax_rule_v = is_syntax_rule<T>::value;

  template<typename T>
  concept syntax_rule = is_syntax_rule_v<T>;

  template<typename Head, syntax_rule ...Rules>
    requires is_syntax_list_v<Head>
  struct syntax_spec {
    using head = Head;
    using rules = std::tuple<Rules...>;
  };

  template<typename T>
  struct is_syntax_spec : std::false_type { };

  template<typename Head, syntax_rule ...Rules>
  struct is_syntax_spec<syntax_spec<Head, Rules...>> : std::true_type { };
  
  template<typename T>
  inline constexpr bool is_syntax_spec_v = is_syntax_spec<T>::value;

  template<typename T>
  concept new_fragment = requires {
    typename T::spec;
    requires is_syntax_spec_v<typename T::spec>;
  };

  template<typename Head, syntax_rule ...Rules>
    requires is_syntax_list_v<Head>
  struct make_new_fragment_t {
    using spec = syntax_spec<Head, Rules...>;
  };

  template<typename Head, syntax_rule ...Rules>
    requires is_syntax_list_v<Head>
  struct make_new_fragment {
    using type = make_new_fragment_t<Head, Rules...>;
  };
  
  //
  // Inclusion of fragments
  //
  template<typename Rules1, typename Rules2>
  struct syntax_rules_imply;

  template<typename Rules1, syntax_rule ...Rules2>
  struct syntax_rules_imply<Rules1, std::tuple<Rules2...>>
    : std::conjunction<syntax_rules_imply<Rules1, Rules2>...> { };

  template<syntax_rule ...Rules1, syntax_rule Rule2>
  struct syntax_rules_imply<std::tuple<Rules1...>, Rule2>
    : std::conjunction<
        syntax_list_includes<
          syntax_list_union_t<typename Rules1::head...>,
          typename Rule2::head
        >,
        syntax_list_includes<
          typename Rule2::children,
          syntax_list_union_t<typename Rules1::children...>
        >
      > { };


  template<typename Spec1, typename Spec2>
  struct syntax_spec_implies 
    : std::conjunction<
        syntax_list_includes<typename Spec2::head, typename Spec1::head>,
        syntax_rules_imply<typename Spec1::rules, typename Spec2::rules>
      > { };


  template<new_fragment Syntax1, new_fragment Syntax2>
  struct is_new_subfragment_of 
    : syntax_spec_implies<typename Syntax1::spec, typename Syntax2::spec> { };

  template<new_fragment Syntax1, new_fragment Syntax2>
  inline constexpr bool is_new_subfragment_of_v
    = is_new_subfragment_of<Syntax1, Syntax2>::value;

  
}

#endif // BLACK_LOGIC_SYNTAX_HPP
