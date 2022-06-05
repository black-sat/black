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

#ifndef BLACK_LOGIC_NAMESPACES_HPP_
#define BLACK_LOGIC_NAMESPACES_HPP_

namespace black::new_api {

  namespace syntax {
    using black::internal::new_api::alphabet;
    using black::internal::new_api::otherwise;
    using black::internal::new_api::syntax_element;
    using black::internal::new_api::make_fragment;
    using black::internal::new_api::make_derived_fragment;
    using black::internal::new_api::make_combined_fragment;

    #define declare_hierarchy(Base) \
      using black::internal::new_api::Base;

    #define declare_storage_kind(Base, Storage) \
      using black::internal::new_api::Storage;

    #define declare_hierarchy_element(Base, Storage, Element) \
      using black::internal::new_api::Element;

    #include <black/new/internal/formula/hierarchy.hpp>
  }

  #define export_in_fragment_namespace(Fragment, Element) \
    namespace Fragment { \
      using Element = \
        black::new_api::syntax::Element<black::new_api::syntax::Fragment>; \
    }

  #define append_syntax_element(Syntax, Element) \
    syntax_element::Element,

  #define using_element(Syntax, Element) \
    using Element = \
      black::internal::new_api::type_for_syntax_element< \
        syntax::Syntax, syntax::syntax_element::Element \
      >;

  #define declare_fragment(Fragment, ...) \
    namespace syntax { \
      struct Fragment : make_fragment< \
        enum_elements_##Fragment(Fragment, append_syntax_element) \
        syntax_element::no_type \
      > { }; \
    } \
    namespace Fragment { \
      using black::internal::new_api::alphabet; \
      using black::internal::new_api::otherwise; \
      enum_elements_##Fragment(Fragment, using_element) \
    }

  #define enum_fragments(Macro, ...) \
    Macro(Boolean, __VA_ARGS__) \
    Macro(FO, __VA_ARGS__) \
    Macro(LTL, __VA_ARGS__) \
    Macro(LTLP, __VA_ARGS__) \
    Macro(LTLFO, __VA_ARGS__) \
    Macro(LTLPFO, __VA_ARGS__) \
 
  #define enum_elements_Boolean(Syntax, Enum) \
    Enum(Syntax, boolean) \
    Enum(Syntax, proposition) \
    Enum(Syntax, negation) \
    Enum(Syntax, conjunction) \
    Enum(Syntax, disjunction) \
    Enum(Syntax, implication) \
    Enum(Syntax, iff)
  
  #define enum_elements_FO(Syntax, Enum) \
    enum_elements_Boolean(Syntax, Enum) \
    Enum(Syntax, atom) \
    Enum(Syntax, exists) \
    Enum(Syntax, forall) \
    Enum(Syntax, constant) \
    Enum(Syntax, variable) \
    Enum(Syntax, application) \
    Enum(Syntax, function_symbol) \
    Enum(Syntax, relation_symbol) \
    Enum(Syntax, negative) \
    Enum(Syntax, subtraction) \
    Enum(Syntax, addition) \
    Enum(Syntax, multiplication) \
    Enum(Syntax, division) \
    Enum(Syntax, equal) \
    Enum(Syntax, not_equal) \
    Enum(Syntax, less_than) \
    Enum(Syntax, less_than_equal) \
    Enum(Syntax, greater_than) \
    Enum(Syntax, greater_than_equal)

  #define enum_elements_LTL(Syntax, Enum) \
    enum_elements_Boolean(Syntax, Enum) \
    Enum(Syntax, tomorrow) \
    Enum(Syntax, w_tomorrow) \
    Enum(Syntax, always) \
    Enum(Syntax, eventually) \
    Enum(Syntax, until) \
    Enum(Syntax, release) \
    Enum(Syntax, w_until) \
    Enum(Syntax, s_release)

  #define enum_elements_LTLP(Syntax, Enum) \
    enum_elements_LTL(Syntax, Enum) \
    Enum(Syntax,yesterday) \
    Enum(Syntax,w_yesterday) \
    Enum(Syntax,once) \
    Enum(Syntax,historically) \
    Enum(Syntax,since) \
    Enum(Syntax,triggered)
  
  #define enum_elements_LTLFO(Syntax, Enum) \
    enum_elements_FO(Syntax, Enum) \
    Enum(Syntax, tomorrow) \
    Enum(Syntax, w_tomorrow) \
    Enum(Syntax, always) \
    Enum(Syntax, eventually) \
    Enum(Syntax, until) \
    Enum(Syntax, release) \
    Enum(Syntax, w_until) \
    Enum(Syntax, s_release) \
    Enum(Syntax, next) \
    Enum(Syntax, wnext) \
    Enum(Syntax, prev) \
    Enum(Syntax, wprev)

  #define enum_elements_LTLPFO(Syntax, Enum) \
    enum_elements_LTLFO(Syntax, Enum) \
    Enum(Syntax,yesterday) \
    Enum(Syntax,w_yesterday) \
    Enum(Syntax,once) \
    Enum(Syntax,historically) \
    Enum(Syntax,since) \
    Enum(Syntax,triggered)

  enum_fragments(declare_fragment)


  #define declare_hierarchy(Base) \
    enum_fragments(export_in_fragment_namespace, Base)

  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    enum_fragments(export_in_fragment_namespace, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  using namespace LTLPFO;

  #undef enum_elements_Boolean
  #undef enum_elements_FO
  #undef enum_elements_LTL
  #undef enum_elements_LTLP
  #undef enum_elements_LTLFO
  #undef enum_elements_LTLPFO
  #undef enum_fragments

  #undef export_in_fragment_namespace
  #undef declare_fragment
  #undef append_syntax_element
  #undef using_element
  
}

#endif // BLACK_LOGIC_NAMESPACES_HPP_
