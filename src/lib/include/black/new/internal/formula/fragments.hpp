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
    struct Element##_hierarchy_type_t { \
      constexpr hierarchy_type type() const { \
        return hierarchy_type::Element; \
      } \
    }; \
    \
    template<typename Derived, bool = true> \
    struct Element##_type_base_t_ { }; \
    \
    template<typename Derived> \
    struct Element##_type_base_t_<Derived, true> { \
      static constexpr Derived Element = Derived{ \
        Element##_hierarchy_type_t{} \
      }; \
    }; \
    \
    template<typename Derived, typename AcceptsType> \
    struct Element##_type_base_t \
      : Element##_type_base_t_< \
          Derived,  \
          AcceptsType::doesit(hierarchy_type::Element) \
    > { };

  #define declare_leaf_storage_kind(Base, Storage) declare_type_t(Storage)
  #define has_no_hierarchy_elements(Base, Storage) declare_type_t(Storage)
  #define declare_hierarchy_element(Base, Storage, Element) \
    declare_type_t(Element)

  #include <black/new/internal/formula/hierarchy.hpp>

  #undef declare_type_t

  #define declare_fragment(Fragment) \
    template<typename Derived, typename AcceptsType> \
    struct Fragment##_type_base_t :
  
  #define allow(Fragment, Element) \
    Element##_type_base_t<Derived, AcceptsType>,

  #define end_fragment(Fragment) \
    dummy_t<Fragment##_type_base_t<Derived, AcceptsType>> { \
      hierarchy_type value; \
    };

  #define declare_derived_fragment(Fragment, Parent) \
    template<typename Derived, typename AcceptsType> \
    struct Fragment##_type_base_t : Parent##_type_base_t<Derived, AcceptsType>,
  
  #define allow_also(Fragment, Element) \
    Element##_type_base_t<Derived, AcceptsType>,

  #define end_derived_fragment(Fragment, Parent) \
    dummy_t<Fragment##_type_base_t<Derived, AcceptsType>> { \
      hierarchy_type value; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_fragment(Fragment) \
    template<typename AcceptsType> \
    struct Fragment##_type \
      : Fragment##_type_base_t<Fragment##_type<AcceptsType>, AcceptsType> \
    { \
      Fragment##_type() = delete; \
      template<typename T> \
      explicit constexpr Fragment##_type(T t) : _type{t.type()} { } \
    \
      hierarchy_type type() const { return _type; } \
    private: \
      hierarchy_type _type; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_fragment(Fragment) \
    using Fragment##_type_list = type_list_remove_last<type_list<

  #define allow(Fragment, Element) hierarchy_type::Element,
    
  #define end_fragment(Fragment) \
    hierarchy_type::no_type>>;

  #define declare_derived_fragment(Fragment, Parent) \
    using Fragment##_type_list = type_list_concat<Parent##_type_list, \
      type_list_remove_last<type_list<

  #define allow_also(Fragment, Element) hierarchy_type::Element,
    
  #define end_derived_fragment(Fragment, Parent) \
    hierarchy_type::no_type>>>;

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_fragment(Fragment) \
    struct Fragment { \
      template<typename AcceptsType> \
      using type = Fragment##_type<AcceptsType>; \
      \
      using list = Fragment##_type_list; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>
}

#endif // BLACK_LOGIC_FRAGMENTS_HPP