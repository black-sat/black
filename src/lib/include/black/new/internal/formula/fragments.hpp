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

#ifndef BLACK_LOGIC_FRAGMENTS_HPP
#define BLACK_LOGIC_FRAGMENTS_HPP

namespace black::internal::new_api {

  #define declare_type_t(Element) \
    struct Element##_syntax_element_t { \
      static constexpr syntax_element type() { \
        return syntax_element::Element; \
      } \
    }; \
    \
    template<bool = true> \
    struct Element##_type_base_t_ { }; \
    \
    template<> \
    struct Element##_type_base_t_<true> { \
      static constexpr Element##_syntax_element_t Element{}; \
    }; \
    \
    template<typename AcceptsType> \
    struct Element##_type_base_t \
      : Element##_type_base_t_< \
          AcceptsType::doesit(syntax_element::Element) \
        > { };

  #define declare_leaf_storage_kind(Base, Storage) declare_type_t(Storage)
  #define has_no_hierarchy_elements(Base, Storage) declare_type_t(Storage)
  #define declare_hierarchy_element(Base, Storage, Element) \
    declare_type_t(Element)

  #include <black/new/internal/formula/hierarchy.hpp>

  #undef declare_type_t

  template<typename AcceptsType, syntax_element Element>
  struct type_base_t;

  template<typename AcceptsType>
  struct type_base_t<AcceptsType, syntax_element::no_type> { };

  #define declare_type_t(Element) \
  template<typename AcceptsType> \
  struct type_base_t<AcceptsType, syntax_element::Element> \
    : Element##_type_base_t<AcceptsType> { };

  #define declare_leaf_storage_kind(Base, Storage) declare_type_t(Storage)
  #define has_no_hierarchy_elements(Base, Storage) declare_type_t(Storage)
  #define declare_hierarchy_element(Base, Storage, Element) \
    declare_type_t(Element)

  #include <black/new/internal/formula/hierarchy.hpp>

  #undef declare_type_t

  template<typename AcceptsType, typename TypeList>
  struct fragment_type_base_t;
  
  template<typename AcceptsType, syntax_element ...Types>
  struct fragment_type_base_t<AcceptsType, type_list<Types...>>
    : type_base_t<AcceptsType, Types>... { };

  template<typename AcceptsType, typename TypeList>
  struct fragment_type 
    : fragment_type_base_t<AcceptsType, TypeList> {
    fragment_type() = delete;
    
    template<typename T, REQUIRES(type_list_contains<TypeList, T::type()>)>
    fragment_type(T) : _type{T::type()} { }

    syntax_element type() const { return _type; }
  private:
    syntax_element _type;
  };

  template<typename TypeList>
  struct type_list_remove_no_type_;
  
  template<typename TypeList>
  using type_list_remove_no_type = 
    typename type_list_remove_no_type_<TypeList>::type;

  template<>
  struct type_list_remove_no_type_<type_list<>> {
    using type = type_list<>;
  };

  template<syntax_element Type, syntax_element ...Types>
  struct type_list_remove_no_type_<type_list<Type, Types...>> {
    using type = type_list_concat<
      type_list<Type>, type_list_remove_no_type<type_list<Types...>>
    >;
  };

  template<syntax_element ...Types>
  struct type_list_remove_no_type_<type_list<syntax_element::no_type, Types...>>
  {
    using type = type_list<Types...>;
  };

  template<syntax_element ...Types>
  struct make_fragment {
    using list = 
      type_list_remove_no_type<type_list_unique<type_list<Types...>>>;
    
    template<typename AcceptsType>
    using type = fragment_type<AcceptsType, list>;
  };

  template<typename Parent, syntax_element ...Types>
  struct make_derived_fragment {
    using list = type_list_remove_no_type<type_list_unique<
      type_list_concat<typename Parent::list, type_list<Types...>>
    >>;

    template<typename AcceptsType>
    using type = fragment_type<AcceptsType, list>;
  };

  template<>
  struct make_combined_fragment_impl<> {
    using list = type_list<>;
    
    template<typename AcceptsType>
    using type = fragment_type<AcceptsType, list>;
  };

  template<typename Syntax>
  struct make_combined_fragment_impl<Syntax> : Syntax { };

  template<typename Syntax, typename ...Syntaxes>
  struct make_combined_fragment_impl<Syntax, Syntaxes...> {
    using list = type_list_unique<
      type_list_concat<
        typename Syntax::list, 
        typename make_combined_fragment_impl<Syntaxes...>::list
      >>;

    template<typename AcceptsType>
    using type = fragment_type<AcceptsType, list>;
  };
}

#endif // BLACK_LOGIC_FRAGMENTS_HPP
