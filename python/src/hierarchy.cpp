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
#include <black/logic/prettyprint.hpp>

#include <pybind11/stl.h>
#include <pybind11/operators.h>

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

  template<black::hierarchy H, typename F>
  void def_bin_op(py::class_<H> &class_, const char *name, F op) {
    using base_t =  internal::hierarchy_type_of_t<syntax, H::hierarchy>;
    class_.def(name, [&](H h, base_t t) {
      return specialize(op(h, t));
    }, py::is_operator());
  };

  template<black::hierarchy H, typename F>
  void def_unary_op(py::class_<H> &class_, const char *name, F op) {
    class_.def(name, [&](H h) {
      return specialize(op(h));
    }, py::is_operator());
  };

  template<typename T>
  void register_operators(T) { }

  template<internal::hierarchy H>
    requires (H::hierarchy == internal::hierarchy_type::term)
  void register_operators(py::class_<H> &class_) {
    def_bin_op(class_, "__add__", std::plus<>{});
    def_bin_op(class_, "__sub__", std::minus<>{});
    def_bin_op(class_, "__mul__", std::multiplies<>{});
    def_bin_op(class_, "__truediv__", std::divides<>{});
    def_unary_op(class_, "__negate__", std::negate<>{});
    def_bin_op(class_, "__eq__", std::equal_to<>{});
    def_bin_op(class_, "__ne__", std::not_equal_to<>{});
    def_bin_op(class_, "__lt__", std::less<>{});
    def_bin_op(class_, "__le__", std::less_equal<>{});
    def_bin_op(class_, "__gt__", std::greater<>{});
    def_bin_op(class_, "__ge__", std::greater_equal<>{});

    if constexpr(H::storage == internal::storage_type::variable) {
      class_.def("__getitem__", [&](H var, black::sort s) {
        return specialize(var[s]);
      });
    }
  }

  template<internal::hierarchy H>
    requires (H::hierarchy == internal::hierarchy_type::formula)
  void register_operators(py::class_<H> &class_) {
    def_unary_op(class_, "__invert__", std::logical_not<>{});
    def_bin_op(class_, "__and__", std::logical_and<>{});
    def_bin_op(class_, "__or__", std::logical_or<>{});
  }
  
  template<internal::hierarchy H>
    requires (H::storage == internal::storage_type::relation)
  void register_operators(py::class_<H> &class_) {
    class_.def("__call__", [](black::relation self, py::args args) {
      std::vector<black::term> terms;
      for(auto it = args.begin(); it != args.end(); ++it) {
        terms.push_back(py::cast<black::term>(*it));
      }
      return black::atom(self, terms);
    });
  }
  
  template<internal::hierarchy H>
    requires (H::storage == internal::storage_type::function)
  void register_operators(py::class_<H> &class_) {
    class_.def("__call__", [](black::function self, py::args args) {
      std::vector<black::term> terms;
      for(auto it = args.begin(); it != args.end(); ++it) {
        terms.push_back(py::cast<black::term>(*it));
      }
      return black::application(self, terms);
    });
  }

  template<typename T>
  void register_api(py::module_ &, T) { }


  template<internal::hierarchy H>
    requires (H::hierarchy == internal::hierarchy_type::formula)
  void register_api(py::module_ &m, py::class_<H> &) {
    m.def("X", [](H h) { return specialize(X(h)); });
    m.def("wX", [](H h) { return specialize(wX(h)); });
    m.def("Y", [](H h) { return specialize(Y(h)); });
    m.def("Z", [](H h) { return specialize(Z(h)); });
    m.def("G", [](H h) { return specialize(G(h)); });
    m.def("F", [](H h) { return specialize(F(h)); });
    m.def("O", [](H h) { return specialize(O(h)); });
    m.def("H", [](H h) { return specialize(H(h)); });
    m.def("implies", [](H h1, H h2) { return specialize(implies(h1, h2)); });
    m.def("U", [](H h1, H h2) { return specialize(U(h1, h2)); });
    m.def("R", [](H h1, H h2) { return specialize(R(h1, h2)); });
    m.def("W", [](H h1, H h2) { return specialize(W(h1, h2)); });
    m.def("M", [](H h1, H h2) { return specialize(M(h1, h2)); });
    m.def("S", [](H h1, H h2) { return specialize(S(h1, h2)); });
    m.def("T", [](H h1, H h2) { return specialize(T(h1, h2)); });
  }


  template<black::syntax_element Element, typename HClass>
  void register_hierarchy_element(
    std::string_view name, py::module_ &m, 
    py::class_<black::alphabet> &alphabet, HClass &hclass
  ) {
    static constexpr auto element = Element;
    static constexpr auto storage = internal::storage_of_element(element);
    static constexpr auto hierarchy = internal::hierarchy_of_storage(storage);

    using element_type = internal::element_type_of_t<syntax, element>;
    // using storage_type = internal::storage_type_of_t<syntax, storage>;
    using hierarchy_type = internal::hierarchy_type_of_t<syntax, hierarchy>;

    py::class_<element_type> class_{m, name.data()};

    hclass.def(py::init<element_type>());

    py::implicitly_convertible<element_type, hierarchy_type>();

    if constexpr(!internal::storage_has_children(storage))
      alphabet.def(
        name.data(), 
        &internal::alphabet_ctor_base<
          element, internal::alphabet_base
        >::construct
      );

    if constexpr(internal::storage_has_children(storage))
      class_.def(
        init_aux<internal::storage_alloc_args_t<syntax, storage>>::init()
      );


    auto declare_fields = [&]<size_t ...I>(std::index_sequence<I...>) {
      (class_.def_property_readonly(
        internal::storage_fields_v<storage>[I].data(), 
        [](element_type self) {
          return specialize(get<I>(self));
        }
      ), ...);
    };

    declare_fields(std::make_index_sequence<std::tuple_size_v<element_type>>{});

    auto repr = [](element_type e) {
      return to_string(hierarchy_type{e});
    };

    class_.def("__str__", repr);
    class_.def("__repr__", repr);

    register_operators(class_);
  }

  template<internal::hierarchy_type Hierarchy>
  void register_hierarchy(
    std::string_view name, py::module_ &m, py::class_<black::alphabet> &alphabet
  ) {
    using hierarchy_type = internal::hierarchy_type_of_t<syntax, Hierarchy>;
    py::class_<hierarchy_type> class_{m, name.data()};

    auto declare = [&]<black::syntax_element E>(black::syntax_list<E>) {
      if constexpr(Hierarchy == hierarchy_of_storage(storage_of_element(E)))
        register_hierarchy_element<E>(to_string(E), m, alphabet, class_);
    };

    auto declare_all = 
      [&]<black::syntax_element ...E>(black::syntax_list<E...>) {
        (declare(black::syntax_list<E>{}), ...);
      };

    declare_all(internal::universal_fragment_t::list{});

    register_api(m, class_);
  }
    
  template<internal::hierarchy_type H>
  bool hierarchy_registered_v = false;

  void register_hierarchies(
    py::module_ &m, py::class_<black::alphabet> &alphabet
  ) {

    auto declare = [&]<black::syntax_element E>(black::syntax_list<E>) {
      static constexpr auto H = hierarchy_of_storage(storage_of_element(E));
      if(!hierarchy_registered_v<H>) {
        register_hierarchy<H>(to_string(H), m, alphabet);
        hierarchy_registered_v<H> = true;
      }
    };

    auto declare_all = 
      [&]<black::syntax_element ...E>(black::syntax_list<E...>) {
        (declare(black::syntax_list<E>{}), ...);
      };

    declare_all(internal::universal_fragment_t::list{});
  }

}
