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
  struct fragment_type_base_t<AcceptsType, syntax_list<Types...>>
    : type_base_t<AcceptsType, Types>... { };

  template<typename AcceptsType, typename TypeList>
  struct fragment_type_t
    : fragment_type_base_t<AcceptsType, TypeList> {
    fragment_type_t() = delete;
    
    template<typename T, REQUIRES(syntax_list_contains_v<TypeList, T::type()>)>
    fragment_type_t(T) : _type{T::type()} { }

    syntax_element type() const { return _type; }
  private:
    syntax_element _type;
  };

  template<typename TypeList>
  struct syntax_list_remove_no_type_;
  
  template<typename TypeList>
  using syntax_list_remove_no_type = 
    typename syntax_list_remove_no_type_<TypeList>::type;

  template<>
  struct syntax_list_remove_no_type_<syntax_list<>> {
    using type = syntax_list<>;
  };

  template<syntax_element Type, syntax_element ...Types>
  struct syntax_list_remove_no_type_<syntax_list<Type, Types...>> {
    using type = syntax_list_concat_t<
      syntax_list<Type>, syntax_list_remove_no_type<syntax_list<Types...>>
    >;
  };

  template<syntax_element ...Types>
  struct syntax_list_remove_no_type_<syntax_list<syntax_element::no_type, Types...>>
  {
    using type = syntax_list<Types...>;
  };

  template<syntax_element ...Types>
  struct make_fragment {
    using list = 
      syntax_list_remove_no_type<syntax_list_unique_t<syntax_list<Types...>>>;
    
    template<typename AcceptsType>
    using type = fragment_type_t<AcceptsType, list>;
  };

  template<typename Parent, syntax_element ...Types>
  struct make_derived_fragment {
    using list = syntax_list_remove_no_type<syntax_list_unique_t<
      syntax_list_concat_t<typename Parent::list, syntax_list<Types...>>
    >>;

    template<typename AcceptsType>
    using type = fragment_type_t<AcceptsType, list>;
  };

  template<typename Syntax1, typename Syntax2>
  struct make_combined_fragment_impl {
    using list = syntax_list_unique_t<
      syntax_list_concat_t<typename Syntax1::list, typename Syntax2::list>
    >;

    template<typename AcceptsType>
    using type = fragment_type_t<AcceptsType, list>;
  };
}

#endif // BLACK_LOGIC_FRAGMENTS_HPP
