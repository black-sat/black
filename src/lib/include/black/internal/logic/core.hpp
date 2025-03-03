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

#ifndef BLACK_LOGIC_SUPPORT_HPP_
#define BLACK_LOGIC_SUPPORT_HPP_

#include <black/support/assert.hpp>
#include <black/support/identifier.hpp>
#include <black/support/bitset.hpp>

#include <functional>
#include <numeric>
#include <ranges>
#include <bitset>
#include <cstdio>
#include <cinttypes>

//
// This file contains all the declarations that do not depend on including the
// hierarchy definition file, i.e. everything that does not need the
// preprocessor. To understand what is happening here, please be sure to know
// how the logic API is used, and then read the comments in `hierarchy.hpp`
// first.
//

namespace black_internal::logic {
  
  //
  // `alphabet` is the main factory class for all the formulas. See its
  // definition for more details.
  //
  class alphabet;
  class alphabet_base;
  
  //
  // This type enums the base hierarchy types like `formula`, `term`, etc...
  // Throughout the code, elements of this enum are used to identify the base 
  // hierarchy type when a precise syntax is not available, so the 
  // corresponding hierarchy class cannot be instantiated, e.g. `formula<S>` is 
  // not available because `S` is not known.
  //
  enum class hierarchy_type : uint8_t;

  //
  // Enumeration similar to `hierarchy_type`, to identify specific storage
  // kinds.
  //
  enum class storage_type : uint8_t;

  //
  // This is the most important enumeration of the system. It lists all the
  // syntactic elements available in the whole hierarchy of types. The elements
  // listed here are all the types in the hierarchy which cannot be further
  // refined into subtypes. These corresponds to leaf storage kinds like
  // `boolean` and `proposition`, hierarchy elements like `conjunction` and
  // `disjunction`, and storage kinds without hierarchy elements such as `atom`.
  //
  enum class syntax_element : uint8_t;

  //
  // This constant bounds the maximum number of items in the `syntax_element`
  // enum. We will ensure later that this is respected.
  //
  inline constexpr size_t syntax_element_max_size = 64;

  //
  // For introspection it is useful to be able to print the values of the above
  // enumerations. Implemented later in the preprocessed code.
  //
  constexpr std::string_view to_string(hierarchy_type);
  constexpr std::string_view to_string(storage_type);
  constexpr std::string_view to_string(syntax_element);

  //
  // A constexpr function to get the base hierarchy_type of a given
  // storage_type. This is only declared here, and defined later by
  // preprocessing the definitions file.
  //
  constexpr hierarchy_type hierarchy_of_storage(storage_type Storage);
  
  //
  // Same thing as above, but to get the `storage_type` of a `syntax_element`
  //
  constexpr storage_type storage_of_element(syntax_element Element);

  //
  // Later we will need to know if a storage type has children. This function
  // fulfills this purpose. Implemented later in the preprocessed code.
  //
  constexpr bool storage_has_children(storage_type storage);
  
  //
  // Last, we get the `syntax_element` of a leaf `storage_type`, or of one that
  // has no hierarchy elements. Note that this is a partial meta-function, since
  // non-leaf storage kinds with hierarchy elements (e.g. `unary`) are not
  // associated with a single `syntax_element`.
  //
  template<storage_type Storage>
  struct element_of_storage;

  template<storage_type Storage>
  inline constexpr auto element_of_storage_v = 
    element_of_storage<Storage>::value;

  //
  // We start to prepare for the declaration of actual hierarchy types such as
  // `formula`, `unary`, `conjunction` etc... Each of these types is a
  // lightweight wrapper over two pointers, one to the alphabet, and one to the
  // actual object that they are a handle for. The type of such object is
  // internal and not intended to be accessed by the user.
  //
  // The following is the base class of all of such types.
  //
  struct hierarchy_node {
    syntax_element type;

    bool operator==(hierarchy_node const&) const = default;
  };

  //
  // This is an opaque type used as a return type for the `unique_id()` member
  // function of the hierarchy types. Useful to compare for identity, hash, or
  // print a unique identifier of hierarchy objects.
  //
  template<hierarchy_type H>
  struct hierarchy_unique_id { 
    enum class type : uintptr_t { };

    friend std::string to_string(type h) {
      constexpr size_t size = sizeof(uintptr_t) * 2 + 1;
      char buf[size];
      snprintf(buf, size, "%" PRIxPTR, static_cast<uintptr_t>(h));
      return buf;
    }
  };

  template<hierarchy_type H>
  using hierarchy_unique_id_t = typename hierarchy_unique_id<H>::type;

  //
  // This concept models the most important types of the system, i.e. hierarchy
  // bases like `formula`, `term`, etc... and all the storage kinds such as
  // `unary`, `binary` and hierarchy elements such as `conjunction`. There are
  // many member types and constexpr members provided, used throughout the
  // system.
  // 1. `T::syntax` is the allowed `fragment`;
  // 2. `T::node_syntax` is the fragment accepted for the current node. While
  //    `T::syntax` talks about the whole subtree rooted at the current node,
  //    `T::node_syntax` talks about the current node only. For example,
  //    `formula<FO>::node_syntax` only accepts the elements from the FO
  //    fragment that are formulas (e.g. no terms). As another example,
  //    `unary<LTL>::node_syntax` only accepts unary operators of LTL.
  // 3. `T::type` is a `fragment_type` instantiated with `T::node_syntax`.
  // 4. `T::hierarchy` is the corresponding `hierarchy_type` 
  //
  // We also model the existence of some member functions, while others are
  // templates and there is no way to check for their existence.
  //
  template<typename T>
  concept hierarchy = requires(T t) {
    { T::hierarchy } -> std::convertible_to<hierarchy_type>;

    { t.unique_id() } -> 
      std::convertible_to<hierarchy_unique_id_t<T::hierarchy>>;
    { t.sigma() } -> std::convertible_to<alphabet *>;
    { t.hash() } -> std::convertible_to<size_t>;
    { t.node_type() } -> std::same_as<syntax_element>;

    // we should constrain the return type of `node()`, but then checking this
    // concept would force the instantiation of `hierarchy_node<>`, which is
    // only defined much later in the preprocessed definition file
    { t.node() };
    
    // missing members:
    // to<H>() function to convert to another compatible hierarchy type H
    // is<H>() function to tell if it can be converted to H
    // match(...) function for pattern matching
  };

  //
  // hierarchy types are hashable
  //
  } namespace std {
    template<black_internal::logic::hierarchy H>
    struct hash<H> {
      size_t operator()(H h) const {
        return h.hash();
      }
    };
  } namespace black_internal::logic {

  //
  // Similarly to hierarchy_node<>, we have storage_node<> as a concrete node
  // type for storage kinds. Its members however must be personalized for each
  // storage kind from the definitions file, so we declare here a trait that
  // will return the concrete type for each element, and will be defined later
  // in the preprocessed part of the code. 
  //
  template<storage_type Storage>
  struct storage_data { };

  template<storage_type Storage>
  using storage_data_t = typename storage_data<Storage>::type;

  //
  // All the specializations of `storage_data` return an instance of the
  // following class template, which simply wraps a tuple of elements. We wrap
  // the tuple because this type will have to be Hashable, but we do not want to
  // define specializations of types of the standard library in a library's
  // header files. So we define the wrapper and provide the instances of
  // `tuple_size`, `tuple_element` and `get<>` for easy access later.
  template<typename ...Types>
  struct make_storage_data_t {
    using tuple_type = std::tuple<Types...>;

    template<typename ...Args>
    make_storage_data_t(Args&& ...args)  
      : values{std::forward<Args>(args)...} { }

    tuple_type values;
  };

  template<typename ...Types>
  struct make_storage_data {
    using type = make_storage_data_t<Types...>;
  };

  //
  // We also provide a preprocessor-friendly version to handle trailing commas
  // properly.
  //
  template<int Dummy, typename ...Types>
  struct make_storage_data_cpp : make_storage_data<Types...> { };

  //
  // Here we define the traits for the tuple-like access to `storage_data_t`
  //
  } namespace std {

    template<typename ...Types>
    struct tuple_size<
      black_internal::logic::make_storage_data_t<Types...>
    > : std::integral_constant<size_t, sizeof...(Types)> { };

    template<size_t I, typename ...Types>
    struct tuple_element<I, 
      black_internal::logic::make_storage_data_t<Types...>
    > : tuple_element<I, tuple<Types...>> { };

  } namespace black_internal::logic {

  //
  // And the function get<> to access the elements
  //
  template<size_t I, typename ...Types>
  std::tuple_element_t<I, make_storage_data_t<Types...>> const&
  get(make_storage_data_t<Types...> const& data) {
    return std::get<I>(data.values);
  }
  
  //
  // To make `make_storage_data_t` hashable, we provide a specialization of
  // std::hash<>
  //
  } namespace std {

    template<typename ...Types>
    struct hash<black_internal::logic::make_storage_data_t<Types...>> 
    {
      size_t operator()(
        black_internal::logic::make_storage_data_t<Types...> const& data
      ) {
        using namespace black_internal;
        using namespace black_internal;

        size_t h = 0;
        std::apply([&]<typename ...Ts>(Ts const& ...values) { // LCOV_EXCL_LINE
          ((h = hash_combine(h, std::hash<Ts>{}(values))), ...);
        }, data.values);

        return h;
      }
    };

  } namespace black_internal::logic {

  //
  // Last thing to make `storage_data_t` hashable is a proper `operator==`
  //
  // However, the standard `operator==` of tuples is not enough. Some hierarchy
  // types, which may end up as types of fields of `make_storage_data_t` (think
  // of the `var` field of `quantifier` which is of type `variable`), might have
  // a custom `operator==` (this happens for `term`s). However, that is defined
  // much later, because we do not have concrete hierarchy types here yet. So we
  // define this helper function that compares everything with the standard
  // `operator==` except for two `hierarchy`s that are compared by
  // `unique_id()`.
  //
  template<hierarchy T1, hierarchy T2>
  bool are_equal(T1 t1, T2 t2) {
    return t1.unique_id() == t2.unique_id();
  }

  template<typename T1, typename T2>
  bool are_equal(T1 t1, T2 t2) {
    return t1 == t2;
  }

  //
  // Now the `operator==` for `make_storage_data_t`. Since we have to zip the
  // two tuples we need an auxiliary function to unpack an index sequence.
  //
  template<size_t ...Idx, typename ...Types>
  bool storage_data_t_cmp_aux(
    make_storage_data_t<Types...> const& k1,
    make_storage_data_t<Types...> const& k2,
    std::index_sequence<Idx...>
  ) {
    return 
      (are_equal(std::get<Idx>(k1.values), std::get<Idx>(k2.values)) && ...);
  }

  template<typename ...Types>
  bool operator==(
    make_storage_data_t<Types...> const& k1,
    make_storage_data_t<Types...> const& k2
  ) {
    return storage_data_t_cmp_aux(k1, k2, std::index_sequence_for<Types...>{});
  }

  //
  // `child_arg` and `children_arg` are the types used to wrap child and
  // children arguments in the construction of storage kinds. Here we only need
  // to forward declare them, and setup a trait to recognize if a type is an
  // instance of `children_arg`. See below for their use and implementation.
  //
  template<hierarchy_type H>
  struct child_arg;
  template<hierarchy_type H>
  struct children_arg;

  template<typename T>
  struct is_children_arg : std::false_type { };

  template<hierarchy_type H>
  struct is_children_arg<children_arg<H>>
    : std:: true_type { };

  template<typename T>
  inline constexpr bool is_children_arg_v =
    is_children_arg<T>::value;

  //
  // Now we can finally declare the actual `storage_node` type, which inherits
  // from `hierarchy_node` through `storage_node_base` and wraps the
  // corresponding `storage_data_t`.
  //
  template<storage_type Storage>
  struct storage_node : hierarchy_node
  { 
    template<typename ...Args>
    storage_node(syntax_element element, Args&& ...args)
      : hierarchy_node{element}, data{std::forward<Args>(args)...} { }

    storage_data_t<Storage> data;

    bool operator==(storage_node const&) const = default;
  };

  //
  // We will use storage nodes themselves as a key in the alphabet's hash tables
  // to retrieve the actual uniqued nodes. Thus we need `storage_node` to be
  // hashable. This is based on the `std::hash` instance of `storage_data_t`,
  // defined above.
  //
  } namespace std {
    template<black_internal::logic::storage_type Storage>
    struct hash<black_internal::logic::storage_node<Storage>> {
      size_t operator()(
        black_internal::logic::storage_node<Storage> const&n
      ) const {
        using namespace black_internal;
        using namespace black_internal::logic;
        
        size_t type_hash = std::hash<syntax_element>{}(n.type);
        size_t data_hash = std::hash<storage_data_t<Storage>>{}(n.data);
        return hash_combine(type_hash, data_hash);
      }
    };
  } namespace black_internal::logic {

  //
  // This concept refines `hierarchy` for storage kinds.
  //
  template<typename T>
  concept storage_kind = hierarchy<T> && requires(T t) {
    { T::storage } -> std::convertible_to<storage_type>;
  };

  //
  // This concept refines `storage_kind` for hierarchy elements.
  //
  template<typename T>
  concept hierarchy_element = storage_kind<T> && requires(T t) {
    { T::element } -> std::convertible_to<syntax_element>;
  };

  //
  // We also forward declare an empty CRTP class that can be specialized by user
  // code to provide custom members to `hierarchy_base` defined below.
  //
  template<hierarchy_type Hierarchy, typename Derived>
  struct hierarchy_custom_members { };

  //
  // We can now declare the base class for all the hierarchy types. This class
  // provides all the necessary infrastructure to obtain a complete hierarchy
  // type. Concrete hierarchy types are obtained by preprocessing the hierarchy
  // definition file by just deriving from this base class and re-exporting the
  // constructors.
  //
  // This is a CRTP base class.
  //
  template<hierarchy_type Hierarchy, typename Derived>
  class hierarchy_base 
  {
  public:
    // member required by the `hierarchy` concept
    static constexpr auto hierarchy = Hierarchy;

    // hierarchy types are not default constructible but are
    // copy/move/constructible/assignable
    hierarchy_base() = delete;
    hierarchy_base(hierarchy_base const&) = default;
    hierarchy_base(hierarchy_base &&) = default;

    hierarchy_base &operator=(hierarchy_base const&) = default;
    hierarchy_base &operator=(hierarchy_base &&) = default;

    // this constructor is for internal use but has to be public (for now)
    hierarchy_base(alphabet_base *sigma, hierarchy_node const*node)
      : _sigma{sigma}, _node{node} 
    { }

    // converting constructor from other hierarchy types
    // the conversion only happen for the same kind of hierarchy (e.g. formulas)
    // and only if the argument's syntax is a subfragment of our syntax
    template<::black_internal::logic::hierarchy H>
      requires (H::hierarchy == Hierarchy)
    hierarchy_base(H h) : hierarchy_base{h.sigma(), h.node()} { }

    //
    // conversion member functions
    //
    template<::black_internal::logic::hierarchy H>
    std::optional<H> to() const {
      return H::from(*this);
    }

    template<::black_internal::logic::hierarchy H>
    bool is() const {
      return to<H>().has_value();
    }

    // this static member function does the job of the `to<>` and `is<>`
    // members. The conversion takes place only if both the `syntax` and the
    // `node_syntax` of the new type agree at runtime with the actual node being
    // converted.
    template<logic::hierarchy F>
    static std::optional<Derived> from(F f) {
      if constexpr(F::hierarchy != Derived::hierarchy)
        return {};

      return f;
    }

    // implemented later
    template<typename ...Handlers>
    auto match(Handlers ...handlers) const;

    auto unique_id() const {
      return hierarchy_unique_id_t<hierarchy>{
        reinterpret_cast<uintptr_t>(_node)
      };
    }

    size_t hash() const {
      return std::hash<hierarchy_node const*>{}(_node);
    }

    syntax_element node_type() const {
      return _node->type;
    }
    
    // we make `sigma()` a function template because `alphabet` is still
    // incomplete at this point, otherwise the static_cast would be invalid.
    template<typename A = alphabet>
    A *sigma() const { return static_cast<A *>(_sigma); }

    hierarchy_node const *node() const { return _node; }

  private:
    alphabet_base *_sigma;
    hierarchy_node const *_node;
  };

  //
  // We can obtain the hierarchy type from the `hierarchy_type` value with the
  // following trait, which will be specialized in the preprocessing code.
  //
  template<hierarchy_type H>
  struct hierarchy_type_of;

  template<hierarchy_type H>
  using hierarchy_type_of_t = typename hierarchy_type_of<H>::type;

  //
  // In general, hierarchy types are equality comparable in a standard way, but
  // the definition file can opt out of it to customize the comparison. What
  // follows set up things to handle that.
  //
  // The first thing is a specializable trait to signal that a hierarchy type
  // has or not a standard equality comparison operator.
  //
  template<hierarchy_type H>
  struct hierarchy_has_standard_equality : std::true_type { };

  template<hierarchy_type H>
  inline constexpr bool hierarchy_has_standard_equality_v =
    hierarchy_has_standard_equality<H>::value;

  //
  // Here we define the standard comparison operators.
  //
  template<hierarchy H1, hierarchy H2>
    requires (H1::hierarchy == H2::hierarchy && 
             hierarchy_has_standard_equality_v<H1::hierarchy>)
  bool operator==(H1 h1, H2 h2) {
    return h1.unique_id() == h2.unique_id();
  }
 
  template<hierarchy H1, hierarchy H2>
    requires (H1::hierarchy == H2::hierarchy && 
             hierarchy_has_standard_equality_v<H1::hierarchy>)
  bool operator!=(H1 h1, H2 h2) {
    return h1.unique_id() != h2.unique_id();
  }

  //
  // We start working for declaring `storage_base` below. We need to first
  // forward-declare it.
  //
  template<storage_type Storage, typename Derived>
  class storage_base;

  //
  // Similarly to `hierarchy_type_of`, we can obtain the concrete storage type
  // of a `storage_type` with this trait, specialized later. Note that the
  // `Syntax` parameter will be ignored if a leaf storage kind is requested.
  //
  template<storage_type H>
  struct storage_type_of;

  template<storage_type H>
  using storage_type_of_t = typename storage_type_of<H>::type;

  //
  // This small trait tells us whether a storage kind has hierarchy elements.
  // It is specialized later by the preprocessed code.
  //
  template<storage_type Storage>
  struct storage_has_hierarchy_elements : std::true_type { };

  template<storage_type Storage>
  inline constexpr bool storage_has_hierarchy_elements_v =
    storage_has_hierarchy_elements<Storage>::value;

  //
  // hierarchy types for storage kinds need to provide access to fields and
  // children. Here we declare two empty CRTP base classes that will be
  // specialized later by the preprocessed hierarchy definition file to provide
  // access to those members.
  //
  template<storage_type Storage, typename Derived>
  struct storage_fields_base { };

  template<storage_type Storage, typename Derived>
  struct storage_children_base { };

  //
  // Similar to `hierarchy_custom_members`, we declare a little empty CRTP class
  // to let anybody add custom members to `storage_base`.
  //
  template<storage_type Storage, typename Derived>
  struct storage_custom_members { };

  //
  // Once we got the arguments into the allocating constructor, we need to build
  // a `storage_node` from them. Since we wrapped the elements of
  // `storage_alloc_args_t` into suitable wrapper types `child_arg` and
  // `children_arg`, which have the right conversion operators, the conversion
  // is automatic, we just have to unpack the tuples correctly.
  //
  // The reason why this function is needed at all is that depending on whether
  // the storage kind has hierarchy elements or not, we need to also pass the
  // `syntax_element` as the first argument to the node constructor.
  //
  template<storage_type Storage, typename ...Args>
    requires storage_has_hierarchy_elements_v<Storage>
  storage_node<Storage> args_to_node(Args ...args) {
    return storage_node<Storage>{std::move(args)...};
  }

  template<storage_type Storage, typename ...Args>
    requires (!storage_has_hierarchy_elements_v<Storage>)
  storage_node<Storage> args_to_node(Args ...args) {
    return 
      storage_node<Storage>{element_of_storage_v<Storage>, std::move(args)...};
  }

  //
  // The `storage_ctor_base` type will be the main base class of `storage_base`
  // below, through which it inherits from `hierarchy_base`. This class takes
  // care of declaring the correct constructor that allocates the nodes from
  // suitable arguments.
  //
  template<
    storage_type Storage, typename Derived, typename Tuple
  >
  class storage_ctor_base;

  template<
    storage_type Storage, typename Derived, typename ...Args
  >
  class storage_ctor_base<Storage, Derived, std::tuple<Args...>> : 
    public hierarchy_base<hierarchy_of_storage(Storage), Derived>
  {
    using base_t = hierarchy_base<hierarchy_of_storage(Storage), Derived>;

  public:
    static constexpr storage_type storage = Storage;

    // the wrapping constructor delegates to the base's one
    storage_ctor_base(alphabet_base *sigma, hierarchy_node const*node) 
      : base_t{sigma, node} 
    { 
      black_assert(storage == storage_of_element(node->type));
    }

    template<typename = void>
      requires (storage_has_hierarchy_elements_v<Storage>)
    explicit storage_ctor_base(syntax_element t, Args ...args) 
      : storage_ctor_base{ 
        get_sigma(args...),
        get_sigma(args...)->unique(storage_node<Storage>{t, std::move(args)...})
      } { }

    template<typename = void>
      requires (!storage_has_hierarchy_elements_v<Storage>)
    explicit storage_ctor_base(Args ...args) 
      : storage_ctor_base{ 
        get_sigma(args...),
        get_sigma(args...)->unique(
          storage_node<Storage>{
            element_of_storage_v<Storage>, std::move(args)...
          }
        )
      } { }

    // we override `node_type()` from `hierarchy_base` to use our `type`.
    syntax_element node_type() const {
      return base_t::node()->type;
    }
  };

  // `storage_ctor_base` needs to be passed a tuple with the arguments that the
  // storage kind expects for the allocation. To know them, we declare a
  // specializable trait, specialized in the preprocessed code.
  template<storage_type Storage>
  struct storage_alloc_args { };

  template<storage_type Storage>
  using storage_alloc_args_t = typename storage_alloc_args<Storage>::type;

  //
  // To declare specializations of `storage_alloc_args` in the preprocessed code
  // we use this maker trait.
  //
  template<int dummy, typename ...Types>
  struct storage_alloc_args_cpp { 
    using type = std::tuple<Types...>;
  };

  //
  // Here we define the base class for storage kinds such as `unary` or
  // `binary`. The constructors allocating the nodes have already been declared
  // in `storage_ctor_base`, which we inherit from.
  //
  // Note that this base class is for non-leaf storage kinds with at least a
  // hierarchy element. Leaf storage kinds (e.g. `boolean`), or storage kinds
  // with no hierarchy elements (e.g. `atom`), are more similar to hierarchy
  // elements and thus inherit from `hierarchy_element_base` declared below.
  // Differently from `hierarchy_base`, this is a CRTP base class.
  //
  template<storage_type Storage, typename Derived>
  class storage_base 
    : public storage_ctor_base<Storage, Derived, storage_alloc_args_t<Storage>>,
      public storage_fields_base<Storage, Derived>,
      public storage_children_base<Storage, Derived>,
      public storage_custom_members<Storage, Derived>
  {
    using base_t = storage_ctor_base<
      Storage, Derived, storage_alloc_args_t<Storage>
    >;
  public:
    storage_base() = default;
    storage_base(storage_base const&) = default;
    storage_base(storage_base &&) = default;

    storage_base &operator=(storage_base const&) = default;
    storage_base &operator=(storage_base &&) = default;

    // we import the constructor(s) from `storage_ctor_base`
    using base_t::base_t;

    // converting constructors from other storages of the same kind. 
    template<storage_kind S>
      requires (S::storage == Storage)
    storage_base(S s) : base_t{s.sigma(), s.node()} { }

    template<logic::hierarchy F>
    static std::optional<Derived> from(F f) {
      if constexpr(F::hierarchy != Derived::hierarchy)
        return {};

      if(storage_of_element(f.node_type()) != Storage)
        return {};

      return Derived{f.sigma(), f.node()};
    }
  };

  //
  // To declare the constructor(s) for `hierarchy_element_base` we need an
  // auxiliary base class similar to `storage_ctor_base`.
  // `hierarchy_element_base` will inherit from `storage_base` through this one.
  //
  template<syntax_element Element, typename D, typename Tuple>
  class hierarchy_element_ctor_base;

  template<syntax_element Element, typename D, typename ...Args>
  class hierarchy_element_ctor_base<Element, D, std::tuple<Args...>> 
    : public storage_base<storage_of_element(Element), D>
  {
    using base_t = storage_base<storage_of_element(Element), D>;

  public:
    // these members from the base have to be overriden and specialized
    static constexpr syntax_element element = Element;

    // the wrapping constructor delegates to the base's one
    hierarchy_element_ctor_base(
      alphabet_base *sigma, hierarchy_node const*node
    ) : base_t{sigma, node} {
      black_assert(element == node->type);
    }

    template<typename = void>
      requires (storage_has_children(storage_of_element(Element)))
    explicit hierarchy_element_ctor_base(Args ...args)
      : hierarchy_element_ctor_base{
        get_sigma(args...),
        get_sigma(args...)->unique(
          args_to_node<base_t::storage>(
            Element, std::move(args)...
          )
        )
      } { }

      // we override `type()` from `hierarchy_base` to use our `type`.
      syntax_element node_type() const {
        return this->node()->type;
      }
  };

  //
  // The following is the last of the three kinds of hierarchy types. Hierarchy
  // elements are the most detailed elements of the hierarchy tree. They are
  // associated to a single `syntax_element` with no more uncertainty. This is a
  // CRTP class as well.
  //
  template<syntax_element Element, typename Derived>
  class hierarchy_element_base
    : public hierarchy_element_ctor_base<
        Element, Derived, storage_alloc_args_t<storage_of_element(Element)>
      >
  {
    using base_t = 
      hierarchy_element_ctor_base<
        Element, Derived, storage_alloc_args_t<storage_of_element(Element)>
      >;

  public:
    hierarchy_element_base() = delete;
    hierarchy_element_base(hierarchy_element_base const&) = default;
    hierarchy_element_base(hierarchy_element_base &&) = default;

    hierarchy_element_base &operator=(hierarchy_element_base const&) = default;
    hierarchy_element_base &operator=(hierarchy_element_base &&) = default;

    using base_t::base_t;

    // converting constructor only from other equal elements with compatible
    // syntax.
    template<hierarchy_element E>
      requires (E::element == Element)
    hierarchy_element_base(E e) 
      : hierarchy_element_base{e.sigma(), e.node()} { }

      template<logic::hierarchy F>
      static std::optional<Derived> from(F f) {
        if constexpr(F::hierarchy != Derived::hierarchy)
          return {};
  
        if(f.node_type() != Element)
          return {};
  
        return Derived{f.sigma(), f.node()};
      }
  };

  //
  // Similarly to `hierarchy_type_of` and `storage_type_of`, we can obtain the
  // concrete type of a `syntax_element` with this trait, specialized later.
  // Note that the `Syntax` parameter will be ignored if a leaf hierarchy
  // element is requested. Note also that the returned type might not be a
  // `hierarchy_element_base` but only a `storage_base`, in the case of leaf
  // storage kinds or storage kinds with no hierarchy element.
  //
  template<syntax_element E>
  struct element_type_of;

  template<syntax_element E>
  using element_type_of_t = typename element_type_of<E>::type;

  //
  // Similarly to `hierarchy_ctor_base`, we declare factory functions to be
  // inherited by `alphabet` for leaf hierarchy types.
  //
  template<syntax_element Element, typename Derived, typename Tuple>
  struct alphabet_ctor_base_aux;

  template<syntax_element Element, typename Derived, typename ...Args>
  struct alphabet_ctor_base_aux<Element, Derived, std::tuple<Args...>>
  {
    using type = element_type_of_t<Element>;
    
    type construct(Args ...args) {
      return type{
        static_cast<Derived *>(this),
        static_cast<Derived *>(this)->unique(
          storage_node<storage_of_element(Element)>{
            Element, std::move(args)...
        }
        )
      };
    }
  };

  template<syntax_element Element, typename Derived>
  struct alphabet_ctor_base 
    : alphabet_ctor_base_aux<
        Element, Derived, storage_alloc_args_t<storage_of_element(Element)>
      > { };

  //
  // Some hierarchy types can appear as arguments both for fields (e.g. the
  // `var` field of `quantifier` which is of type `variable`), and for children.
  // In the second case, since we must handle the fragments etc.., we store the
  // pointer to the underlying node. In the first case we do not. To distinguish
  // these cases, we declare here small wrapper types that will convert to the
  // appropriate type later.
  //
  template<hierarchy_type Hierarchy>
  struct child_arg 
  {
    template<hierarchy H>
      requires (H::hierarchy == Hierarchy)
    child_arg(H h) : child{h.node()}, sigma{h.sigma()} { }

    operator hierarchy_node const*() const {
      return child;
    }
    
    hierarchy_node const *child;
    alphabet *sigma;
  };

  template<hierarchy_type H>
  struct children_arg
  {
    template<std::ranges::range R>
    using value_t = std::ranges::range_value_t<R>;

    template<std::ranges::range R>
      requires (value_t<R>::hierarchy == H)
    children_arg(R v) {
      black_assert(!empty(v));
      sigma = begin(v)->sigma();
      for(auto h : v)
        children.push_back(h.node());
    }

    operator std::vector<hierarchy_node const*>() const {
      return children;
    }

    std::vector<hierarchy_node const*> children;
    alphabet *sigma;
  };

  //
  // The allocating constructor needs to get access to the alphabet to make any
  // allocation. The alphabet is extracted with the following function from the
  // first argument that is a `hierarchy` or a range of `hierarchy`s. If no such
  // argument exists it means the hierarchy is a leaf and an instance has to be
  // requested directly to the alphabet (e.g. sigma.proposition("p")).
  //
  template<hierarchy_type H, typename ...Args>
  alphabet *get_sigma(child_arg<H> child, Args ...) {
    return child.sigma;
  }
  template<typename T, typename ...Args>
    requires (is_children_arg_v<std::remove_cvref_t<T>>)
  alphabet *get_sigma(T&& children, Args ...) {
    return children.sigma;
  }

  template<typename H, typename ...Args>
    requires hierarchy<std::remove_cvref_t<H>>
  alphabet *get_sigma(H&& h, Args...) {
    return h.sigma();
  }

  template<typename T, typename ...Args>
  alphabet *get_sigma(T&&, Args ...args) {
    return get_sigma(args...);
  }

  //
  // Let us now address the remaining parts of the user interface of hierarchy
  // types. `storage_kind`s provides the user with member functions to access
  // fields and children (e.g. left() and right(), or name()). In the
  // preprocessing code, we only have the name of those fields, while the
  // contents are stored in tuples which can be accessed by index. Hence we need
  // a map from names of fields to indices in the tuples. This is implemented as
  // just an array of string views declared in the preprocessed code. Then, a
  // bunch of constexpr symbols holding the name of the single fields is
  // declared. Here, we only need to declare a function that searches for one
  // symbol in the table and returns its index. Note that the specialization of
  // the trait is selected with SFINAE instead of a `requires` clause because of
  // a bug in GCC 10.
  template<
    size_t I, std::string_view const*lits, const char *literal, typename = void
  >
  struct index_of_field : index_of_field<I + 1, lits, literal> { };

  template<size_t I, std::string_view const*lits, const char *literal>
  struct index_of_field<
    I, lits, literal, std::enable_if_t<lits[I] == literal>
  > : std::integral_constant<size_t, I> { };

  template<std::string_view const*lits, const char *literal>
  inline constexpr size_t index_of_field_v = 
    index_of_field<0, lits, literal>::value;

  //
  // Once we found which field to get, we just wrap it if needed. In order to
  // wrap child and children fields, we need to know their `hierarchy_type`.
  // This trait does this, specialized later.
  //
  template<size_t I, storage_type Storage>
  struct hierarchy_of_storage_child;
  
  template<size_t I, storage_type Storage>
  inline constexpr auto hierarchy_of_storage_child_v =
    hierarchy_of_storage_child<I, Storage>::value;
  
  //
  // The following is a trait template to get an array with the actual names
  // of the fields of a storage, specialized later.
  //
  template<storage_type>
  struct storage_fields {
    static constexpr auto value = nullptr;
  };

  template<storage_type Storage>
  inline constexpr auto storage_fields_v = storage_fields<Storage>::value;

  //
  // Now we can actually get the fields.
  //
  template<size_t I, storage_kind H>
  auto const &get_field(H h) {
    return std::get<I>(
      static_cast<storage_node<H::storage> const *>(h.node())->data.values
    );
  }

  template<size_t I, storage_kind H>
  auto get_child(H h) {
    using ChildH = 
      hierarchy_type_of_t<hierarchy_of_storage_child_v<I, H::storage>>;
    return ChildH{
      h.sigma(), 
      std::get<I>(
        static_cast<storage_node<H::storage> const *>(h.node())->data.values
      )
    };
  }

  template<size_t I, storage_kind H>
  auto get_children(H h) {
    using ChildH = 
      hierarchy_type_of_t<hierarchy_of_storage_child_v<I, H::storage>>;
    alphabet *sigma = h.sigma();
    auto children = std::get<I>(
      static_cast<storage_node<H::storage> const *>(h.node())->data.values
    );

    std::vector<ChildH> result;
    for(auto child : children)
      result.push_back(ChildH{sigma, child});

    return result;
  }

  //
  // Another piece of public interface is the tuple-like access to hierarchy
  // types. This allows one to do something like `auto [l, r] = c;` to get left
  // and right children of a conjunction, and is also used by the pattern
  // matching infrastructure defined later.
  //
  // First, we need to know if the last child is a vector of children.
  // Specialized later.
  //
  template<size_t I, storage_type Storage>
  struct storage_ith_data_is_field : std::false_type { };
  
  template<size_t I, storage_type Storage>
  inline constexpr bool storage_ith_data_is_field_v =
    storage_ith_data_is_field<I, Storage>::value;
  
  template<size_t I, storage_type Storage>
  struct storage_ith_data_is_child : std::false_type { };
  
  template<size_t I, storage_type Storage>
  inline constexpr bool storage_ith_data_is_child_v =
    storage_ith_data_is_child<I, Storage>::value;

  //
  // The implementation of get<> calls get_field(), get_child() or
  // get_children() depending on the index requested.
  //
  template<size_t I, storage_kind S>
  auto get(S s) {
    if constexpr(storage_ith_data_is_field_v<I, S::storage>)
      return get_field<I>(s);
    else if constexpr(storage_ith_data_is_child_v<I, S::storage>)
      return get_child<I>(s);
    else 
      return get_children<I>(s);
  }

  //
  // Then we can declare the specializations.
  //
  } namespace std {

    template<black_internal::logic::storage_kind S>
    struct tuple_size<S> 
      : tuple_size<black_internal::logic::storage_data_t<S::storage>> { };

    template<size_t I, black_internal::logic::storage_kind S>
    struct tuple_element<I, S> {
      using type = decltype(get<I>(std::declval<S>()));
    };

  } namespace black_internal::logic {

  //
  // We also declare a function that, based on get<> above, makes a tuple of all
  // the fields of a hierarchy object. This is useful in combination with
  // `std::apply` which currently does not support Tuple-like user-defined
  // types.
  //
  template<storage_kind S, size_t ...I>
  auto as_tuple(S s, std::index_sequence<I...>) {
    return std::tuple{get<I>(s)...};
  }

  template<storage_kind S>
  auto as_tuple(S s) {
    return as_tuple(s, std::make_index_sequence<std::tuple_size_v<S>>{});
  }

  //
  // Before going into the implementation of the pattern matching
  // infrastructure, we implement the facility that it will use to compute the
  // return type of the `match()` function when the cases lambdas return
  // hierarchy types. We reuse the `std::common_type` trait, and we specialize
  // it for our types. The actual `std` specializations are in the preprocessed
  // source, since we have to provide it for actual user-defined types.
  //
  // The common type between two hierarchy types only exists of course if the
  // hierarchy is the same (e.g. formulas to formulas). The common type is the
  // hierarchy type with the combined fragment. Note that by the way
  // `make_combined_fragment_t` works, this handles the case where one of the
  // two fragments is subsumed by the other, returning directly the more general
  // one. Thus, `std::common_type_t<formula<LTL>, formula<propositional>>` is
  // `formula<LTL>`, while `std::common_type_t<formula<LTL>, formula<FO>>` is
  // equivalent to `formula<LTLFO>`.
  template<typename T, typename U>
  struct common_type_helper { };

  //
  // Little helper template used later
  //
  template<typename T, typename H>
  struct have_same_storage : std::false_type { };
  
  template<storage_kind T, storage_kind H>
    requires (T::storage == H::storage)
  struct have_same_storage<T,H> : std::true_type { };

  template<typename T, typename H>
  inline constexpr bool have_same_storage_v = have_same_storage<T,H>::value;

  //
  // This is the common type of two hierarchies that are not storage kinds of
  // the same storage type.
  //
  template<hierarchy T, hierarchy U>
    requires (T::hierarchy == U::hierarchy && !have_same_storage_v<T, U>)
  struct common_type_helper<T, U> 
    : hierarchy_type_of<T::hierarchy> { };
  
  template<storage_kind T, storage_kind U>
    requires (T::storage == U::storage)
  struct common_type_helper<T, U> 
    : storage_type_of<T::storage> { };

  //
  // The actual specialization
  //
  } namespace std {

    template<
      black_internal::logic::hierarchy T, 
      black_internal::logic::hierarchy U
    > struct common_type<T, U>
      : black_internal::logic::common_type_helper<T, U> { };

  } namespace black_internal::logic {

  //
  // Here we declare the infrastructure for pattern matching. The machinery is
  // based on the to<> function of hierarchy types, and on the list of
  // `syntax_element`s provided by the hierarchy's fragment. For each syntax
  // element we try to cast the formula to the corresponding type using
  // `to<>()`, calling the corresponding lambda case if the cast succeeds. This
  // is all made more complex because of the support for tuple-unpacking the
  // hierarchy type to the lambda arguments.
  //
  // The first thing we need is a function to do this std::apply-like unpacking
  // of the hierarchy to the called lambda. 

  template<typename Handler, typename T, size_t ...I>
    requires std::invocable<Handler, T, std::tuple_element_t<I, T>...>
  auto unpack(
    Handler&& handler, T h, std::index_sequence<I...>
  ) {
    return std::invoke(std::forward<Handler>(handler), h, get<I>(h)...);
  }

  //
  // It is cumbersome to repeat everything both in the body and in `decltype()`
  // but we need to remove the function from overload resolution if the handler
  // is not callable.
  //
  template<typename Handler, typename T>
  auto unpack(Handler&& handler, T h)
  -> decltype(
    unpack(
      std::forward<Handler>(handler), h, 
      std::make_index_sequence<std::tuple_size_v<T>>{}
    )
  ) {
    return unpack(
      std::forward<Handler>(handler), h, 
      std::make_index_sequence<std::tuple_size_v<T>>{}
    );
  }

  //
  // This trait holds if the call to `unpack` is well-formed
  //
  template<typename H, typename Handler>
  inline constexpr bool can_be_unpacked_v = requires { 
    unpack(std::declval<Handler>(), std::declval<H>()); 
  };

  struct unexhaustive_pattern_t {
    template<typename T>
    [[noreturn]] explicit operator T() { black_unreachable(); }
  };

} namespace std {

  template<typename T>
  struct std::common_type<T, black_internal::logic::unexhaustive_pattern_t>
    : std::type_identity<T> { };

  template<typename T>
  struct std::common_type<black_internal::logic::unexhaustive_pattern_t, T>
    : std::type_identity<T> { };
  
  template<>
  struct std::common_type<
    black_internal::logic::unexhaustive_pattern_t,
    black_internal::logic::unexhaustive_pattern_t
  >
    : std::type_identity<black_internal::logic::unexhaustive_pattern_t> { };

} namespace black_internal::logic {

  //
  // The `dispatch()` function takes a hierarchy object and the list of handlers
  // and calls the first handler that can be called either directly or by
  // unpacking. Note that we check the unpacked case before to give it priority,
  // so that e.g. a lambda such as `[](conjunction, auto ...args) { }` picks up
  // the unpacked children in `args`.
  //
  template<typename T, typename Handler, typename ... Handlers>
  auto dispatch(T obj, Handler&& handler, Handlers&& ...handlers) 
  {
    if constexpr(can_be_unpacked_v<T, Handler>) 
      return unpack(std::forward<Handler>(handler), obj);
    else if constexpr(std::is_invocable_v<Handler, T>)
      return std::invoke(std::forward<Handler>(handler), obj);
    else 
      return dispatch(obj, std::forward<Handlers>(handlers)...);
  }

  template<typename T>
  auto dispatch(T) {
    return unexhaustive_pattern_t{};
  }

  //
  // Finally, the `matcher` class, which calls the machinery above to do the
  // trick. This is generic and works not only for hierarchy types but for any
  // type that expose `to<>` and `is<>` functions (e.g. `fragment_type` above).
  //
  // At first we need a concept to test the usability of such functions
  //
  template<typename H, typename Case>
  concept can_cast_to = requires(H h) {
    { h.template to<Case>() } -> std::convertible_to<std::optional<Case>>;
    { h.template is<Case>() } -> std::convertible_to<bool>;
  };

  //
  // To be defined later
  //
  template<typename>
  struct every_syntax_element;
  
  template<typename Dummy>
  using every_syntax_element_t = typename every_syntax_element<Dummy>::type;

  //
  // Now the matcher class itself. The `H` paremeter is the main class from
  // which one wants to match (e.g. `formula<Syntax>`). The `Cases` parameter is
  // a tuple of types to try to match the matched object to. The default value
  // for `Cases` is the correct one to use in the common case of `H` being a
  // hierarchy type.
  //
  template<typename H, typename Cases = every_syntax_element_t<H>>
  struct matcher;

  template<typename H, typename Case, typename ...Cases>
    //requires can_cast_to<H, Case>
  struct matcher<H, std::tuple<Case, Cases...>>
  {
    template<typename ...Handlers>
    using return_type = std::common_type_t<
      decltype(dispatch(
        *std::declval<H>().template to<Case>(), std::declval<Handlers>()...
      )),
      decltype(matcher<H, std::tuple<Cases...>>::match(
        std::declval<H>(), std::declval<Handlers>()...
      ))
    >;

    //
    // The return type of `match()` is computed with `std::common_type`, which
    // has been specialized for hierarchies above. Again, it is cumbersome to
    // repeat the body twice but there's no other way.
    //
    template<typename ...Handlers, typename R = return_type<Handlers...>>
    static R match(H h, Handlers&& ...handlers)
    {
      if(h.template is<Case>())
        return static_cast<R>(
          dispatch(
            *h.template to<Case>(), std::forward<Handlers>(handlers)...
          )
        );
      
      return static_cast<R>(
        matcher<H, std::tuple<Cases...>>::match(
          h, std::forward<Handlers>(handlers)...
        )
      );
    }
  };

  //
  // The base case of the recursion above is the singleton list of cases. If we
  // reach this, and the main type cannot be casted to this last case, it means
  // it could not have been casted to any of the elements included in the list
  // of cases, which is clearly a bug, so we raise an assertion.
  //
  template<typename H, typename Case>
    //requires can_cast_to<H, Case>
  struct matcher<H, std::tuple<Case>>
  {
    template<typename ...Handlers>
    static auto match(H h, Handlers&& ...handlers) 
      -> decltype(dispatch(
        *h.template to<Case>(), std::forward<Handlers>(handlers)...
      )) 
    {
      if(h.template is<Case>())
        return dispatch(
          *h.template to<Case>(), std::forward<Handlers>(handlers)...
        );
      black_unreachable(); // LCOV_EXCL_LINE
    }
  };

  //
  // Then for hierarchy types
  //
  template<hierarchy_type H, typename Derived>
  template<typename ...Handlers>
  auto hierarchy_base<H, Derived>::match(Handlers ...handlers) const {
    return matcher<Derived>{}.match(
      static_cast<Derived const&>(*this), handlers...
    );
  }

  //
  // Little catch-all type used as a wildcard in patter matching 
  //
  struct otherwise {
    template<typename T>
    otherwise(T const&) { }
  };

  //
  // Here we start defining a set of utilities to work with hierarchy types and
  // objects.
  //
  // The first is a function to orchestrate a generic recursion over all the
  // children of a hierarchy. When the concrete hierarchy is known this is
  // trivial, with a recursive function and the `match()` function, but when the
  // hierarchy type is generic, traversing all the children can be cumbersome.
  // With this function, it's easier. We apply a function to each child of the
  // hierarchy object, and that's it. We do not record the return value of the
  // function, because there is no way to return it back: in case of a known
  // storage kind, we know in particular which children are there and we could
  // return a tuple of the results, but when called on a generic hierarchy
  // object, there is no way to know which type to return. So the called
  // function must imperatively do something on its captured variables, but this
  // is not an heavy limitation.
  //
  // At first we define the overload of the function that takes a
  // `storage_kind`, since we know exactly which children there are in this
  // case. 
  template<size_t I, storage_type S, typename F>
  void for_each_child_aux(F) { }
  
  template<size_t I, storage_type S, typename Arg, typename ...Args, typename F>
  void for_each_child_aux(F f, Arg arg, Args...args) 
  {
    if constexpr(storage_ith_data_is_field_v<I, S>)
      for_each_child_aux<I+1, S>(f, args...);
    else if constexpr(storage_ith_data_is_child_v<I, S>) {
      f(arg);
      for_each_child_aux<I+1, S>(f, args...);
    } else {
      for(auto child : arg)
        f(child);
      for_each_child_aux<I+1, S>(f, args...);
    }
  }

  template<storage_kind S, typename F>
  void for_each_child(S s, F f) {
    std::apply([&](auto ...fields) {
      for_each_child_aux<0, S::storage>(f, fields...);
    }, as_tuple(s));
  }

  //
  // Then, we declare the overload that takes a `hierarchy`, which has to look
  // at which storage kind the hierarchy is in particular and call the
  // `storage_kind` overload consequently. This is done simply by reusing the
  // `match()` function. The lambda cases are called with the hierarchy
  // downcasted to the single hierarchy elements, so inside a generic lambda we
  // have the concrete type to call the `storage_kind` overload of the function.
  //
  template<hierarchy H, typename F>
  void for_each_child(H h, F f) {
    h.match(
      [&](auto s) {
        for_each_child(s, f);
      }
    );
  }

  //
  // Recursive version of for_each_child, going all deep down the rabbit hole...
  //
  template<hierarchy H, typename F>
  void for_each_child_deep(H h, F f) {
    for_each_child(h, [&](auto child) {
      f(child);
      for_each_child_deep(child, f);
    });
  }

  //
  // A good application for `for_each_child`. This function tells whether a
  // hierarchy object `h` contains any element among those given as arguments.
  // For example, if `f` is a formula, `has_any_element_of(f,
  // syntax_element::boolean, syntax_element::iff)` tells whether there is any
  // boolean constant in the formula or any double implication.
  //
  template<hierarchy H, typename ...Args>
    requires (std::is_constructible_v<syntax_element, Args> && ...)
  bool has_any_element_of(H h, Args ...args) {
    if(((h.node_type() == args) || ...))
      return true;
    
    bool has = false;
    for_each_child(h, [&](auto child) {
      if(has_any_element_of(child, args...))
        has = true;
    });

    return has;
  }
}

#endif // BLACK_LOGIC_SUPPORT_HPP_
