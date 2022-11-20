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

#include <black/python/hierarchy.hpp>
#include <black/python/support.hpp>

#include <black/logic/logic.hpp>

#include <pybind11/stl.h>

#include <vector>
#include <tuple>
#include <variant>

namespace pyblack 
{
  namespace internal = black_internal::logic;

  using syntax = black::logic::LTLPFO;

  template<typename T>
  struct clean_init_arg {
    using type = T;
  };

  template<internal::hierarchy_type H>
  struct clean_init_arg<internal::child_arg<H, syntax>>
    : internal::hierarchy_type_of<syntax, H> { };
  
  template<internal::hierarchy_type H>
  struct clean_init_arg<internal::children_arg<H, syntax>> {
    using type = std::vector<internal::hierarchy_type_of_t<syntax, H>>;
  };

  template<typename T>
  using clean_init_arg_t = typename clean_init_arg<T>::type;

  template<typename Tuple>
  struct init_aux;
  
  template<typename ...Args>
  struct init_aux<std::tuple<Args...>> {
    static auto init() {
      return py::init<clean_init_arg_t<Args>...>();
    }
  };

  template<typename List>
  struct make_universal_variant;

  template<black::syntax_element ...Elements>
  struct make_universal_variant<black::syntax_list<Elements...>> {
    using type = std::variant<internal::element_type_of_t<syntax, Elements>...>;
  };

  using universal_variant_t = typename 
    make_universal_variant<internal::universal_fragment_t::list>::type;

  template<typename T>
  inline auto specialize(T&& t) {
    return t;
  }

  template<typename H>
    requires black::hierarchy<std::remove_cvref_t<H>>
  inline auto specialize(H&& h) {
    return h.match(
      [](auto x) {
        return universal_variant_t{x};
      }
    );
  }

  template<black::syntax_element Element, typename HClass>
  void register_hierarchy_element(
    const char *name, py::module_ &m, py::class_<black::alphabet> &alphabet,
    HClass &hclass
  ) {
    static constexpr auto element = Element;
    static constexpr auto storage = internal::storage_of_element(element);
    static constexpr auto hierarchy = internal::hierarchy_of_storage(storage);

    using element_type = internal::element_type_of_t<syntax, element>;
    // using storage_type = internal::storage_type_of_t<syntax, storage>;
    using hierarchy_type = internal::hierarchy_type_of_t<syntax, hierarchy>;

    py::class_<element_type> class_{m, name};

    hclass.def(py::init<element_type>());

    py::implicitly_convertible<element_type, hierarchy_type>();

    if constexpr(!internal::storage_has_children(storage))
      alphabet.def(
        name, 
        &internal::alphabet_ctor_base<
          element, internal::alphabet_base
        >::construct
      );

    if constexpr(internal::storage_has_children(storage))
      class_.def(
        init_aux<internal::storage_alloc_args_t<syntax, storage>>::init()
      );

    static constexpr size_t n_fields = std::tuple_size_v<element_type>;

    auto declare_fields = [&]<size_t ...I>(std::index_sequence<I...>) {
      (class_.def_property_readonly(
        internal::storage_fields_v<storage>[I].data(), 
        [](element_type self) {
          return specialize(get<I>(self));
        }
      ), ...);
    };

    declare_fields(std::make_index_sequence<n_fields>{});
  }

  template<internal::hierarchy_type Hierarchy>
  void register_hierarchy(
    const char *name, py::module_ &m, py::class_<black::alphabet> &alphabet
  ) {
    using hierarchy_type = internal::hierarchy_type_of_t<syntax, Hierarchy>;
    py::class_<hierarchy_type> class_{m, name};

    #define declare_leaf_storage_kind(Base, Storage) \
      declare_hierarchy_element(Base, Storage, Storage)

    #define has_no_hierarchy_elements(Base, Storage) \
      declare_hierarchy_element(Base, Storage, Storage)

    #define declare_hierarchy_element(Base, Storage, Element) \
      if constexpr(Hierarchy == internal::hierarchy_type::Base) \
        register_hierarchy_element<black::syntax_element::Element>( \
          #Element, m, alphabet, class_ \
        );

    #include <black/internal/logic/hierarchy.hpp>
  }
    

  void register_hierarchies(
    py::module_ &m, py::class_<black::alphabet> &alphabet
  ) {
    #define declare_hierarchy(Base) \
      register_hierarchy<internal::hierarchy_type::Base>(#Base, m, alphabet);

    #include <black/internal/logic/hierarchy.hpp>
  }

}
