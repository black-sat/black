//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_AST_CORE_HPP
#define BLACK_AST_CORE_HPP

#include <string_view>
#include <source_location>
#include <tuple>

//
// Public interface for reflection of AST nodes.
//
// See reflect.hpp to see how these are implemented from the expansion 
// of defs.hpp
//
namespace black::ast {

  template<typename AST>
  struct is_ast : std::false_type { };
  
  template<typename AST>
  inline constexpr bool is_ast_v = is_ast<AST>::value;

  template<typename AST>
  concept ast = is_ast_v<AST>;
  
  template<typename Node>
  struct is_ast_node : std::false_type { };
  
  template<typename Node>
  inline constexpr bool is_ast_node_v = is_ast_node<Node>::value;

  template<typename Node>
  concept ast_node = is_ast_node_v<Node>;

  template<typename Node, ast AST>
  struct is_ast_node_of : std::false_type { };
  
  template<typename Node, ast AST>
  inline constexpr bool is_ast_node_of_v = is_ast_node_of<Node, AST>::value;

  template<typename Node, typename AST>
  concept ast_node_of = ast<AST> && is_ast_node_of_v<Node, AST>;

  template<ast_node Node>
  struct ast_of { };
  
  template<ast_node Node>
  using ast_of_t = typename ast_of<Node>::type;

  template<ast AST>
  struct ast_node_list { };
  
  template<ast AST>
  using ast_node_list_t = typename ast_node_list<AST>::type;

  template<ast AST>
  struct ast_node_index { };

  template<ast AST>
  using ast_node_index_t = typename ast_node_index<AST>::type;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_index_of { };
  
  template<ast AST, ast_node_of<AST> Node>
  inline constexpr auto ast_node_index_of_v = 
    ast_node_index_of<AST, Node>::value;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_name { };

  template<ast AST, ast_node_of<AST> Node>
  inline constexpr auto ast_node_name_v = ast_node_name<AST, Node>::value;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_field_index { };
  
  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_index_t = typename ast_node_field_index<AST, Node>::type;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_field_list { };

  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_list_t = typename ast_node_field_list<AST, Node>::type;

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  struct ast_node_field_type { };

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  using ast_node_field_type_t = 
    typename ast_node_field_type<AST, Node, Field>::type;

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  struct ast_node_field_name { };
  
  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  inline constexpr auto ast_node_field_name_v = 
    ast_node_field_name<AST, Node, Field>::value;

  template<
    typename Derived,
    ast AST, ast_node_of<AST> Node,
    auto Member
  >
  struct ast_node_member_base { };
  
  template<
    typename Derived,
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field,
    auto Member
  >
  struct ast_node_field_member_base { };

  template<typename Derived, ast AST>
  struct ast_factory_named_member { };

  //
  // The following traits do not need x-expansion to be implemented
  //
  template<
    ast AST, ast_node_of<AST> Node, 
    typename = ast_node_field_list_t<AST, Node>
  >
  struct ast_node_field_types { };

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> ...Fields
  >
  struct ast_node_field_types<
    AST, Node, 
    std::tuple<
      std::integral_constant<ast_node_field_index_t<AST, Node>, Fields>...
    >
  > : std::type_identity<
        std::tuple<ast_node_field_type_t<AST, Node, Fields>...>
      > { };

  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_types_t = typename ast_node_field_types<AST, Node>::type;

}

//
// Implementation of AST nodes
//
namespace black::ast::internal {

  template<ast AST>
  struct ast_impl_base {
    const ast_node_index_t<AST> index;

    bool operator==(ast_impl_base const&) const = default;
  };

  template<ast AST>
  using ast_impl_ref = std::shared_ptr<ast_impl_base<AST> const>;
  
  template<ast AST>
  using ast_impl_weak_ref = std::weak_ptr<ast_impl_base<AST> const>;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_impl : ast_impl_base<AST> {
    
    ast_impl(ast_node_field_types_t<AST, Node> _data) 
      : ast_impl_base<AST>{ast_node_index_of_v<AST, Node>}, data{_data} { }
    
    bool operator==(ast_impl const&) const = default;

    const ast_node_field_types_t<AST, Node> data;
  };

  template<ast AST>
  struct ast_factory;

  enum class unique_id_t : uintptr_t { };
  
  inline bool operator==(unique_id_t id1, unique_id_t id2) {
    return uintptr_t(id1) == uintptr_t(id2);
  }
  
  inline bool operator!=(unique_id_t id1, unique_id_t id2) {
    return uintptr_t(id1) != uintptr_t(id2);
  }

  template<ast AST>
  class node_holder {
    public:
      size_t hash() const { return support::hash(_factory, _impl); }
    
      unique_id_t unique_id() const { 
        return unique_id_t(reinterpret_cast<uintptr_t>(_impl.get()));
      }

    protected:
      ast_factory<AST> *factory() const { return _factory; }
      ast_impl_ref<AST> impl() const { return _impl; }

      node_holder(ast_factory<AST> *f, ast_impl_ref<AST> i)
        : _factory{f}, _impl{i} { }

      template<
        ast A, ast_node_of<A>, size_t, 
        std::derived_from<node_holder<A>> T, typename ...Others
      >
      friend ast_factory<A> *factory_of(
        std::source_location, T const& h, Others...
      );

      template<ast A, ast_node_of<AST> N, ast_node_field_index_t<A, N>>
      friend struct ast_node_fields_base;

      ast_factory<AST> *_factory;
      ast_impl_ref<AST> _impl;
  };

  template<
    ast AST, ast_node_of<AST> Node, typename = ast_node_field_types_t<AST, Node>
  >
  struct ast_node_base;

  template<ast AST>
  struct ast_base 
    : node_holder<AST>, ast_factory_named_member<AST, AST>
  {
    using node_holder<AST>::node_holder;

    ast_base() = delete;
    ast_base(ast_base const&) = default;
    ast_base(ast_base &&) = default;
    
    template<ast_node_of<AST> Node>
    ast_base(Node n) 
      : node_holder<AST>(n._factory, n._impl) { }

    ast_base &operator=(ast_base const&) = default;
    ast_base &operator=(ast_base &&) = default;

    template<ast_node_of<AST> Node>
    std::optional<Node> to() const {
      if(this->_impl->index == ast_node_index_of_v<AST, Node>)
        return Node{this->_factory, this->_impl};
      
      return {};
    }

    template<ast_node_of<AST> Node>
    bool is() const {
      return to<Node>().has_value();
    }

    template<ast A, ast_node_of<A>, typename>
      friend struct ast_node_base;
  };

  template<ast AST>
  bool operator==(AST t1, AST t2) {
    return t1.unique_id() == t2.unique_id();
  }

  template<ast AST>
  bool operator!=(AST t1, AST t2) {
    return t1.unique_id() != t2.unique_id();
  }

  template<ast AST, typename T>
  struct has_factory : std::false_type { };
  
  template<ast AST, typename T>
  inline constexpr bool has_factory_v = has_factory<AST, T>::value;

  template<ast AST>
  struct has_factory<AST, AST> : std::true_type { };
  
  template<ast AST, ast_node_of<AST> Node>
  struct has_factory<AST, Node> : std::true_type { };

  template<ast AST, std::ranges::range R>
  struct has_factory<AST, R> 
    : has_factory<AST, std::ranges::range_value_t<R>> { };

  template<
    ast AST, ast_node_of<AST> Node, typename = ast_node_field_types_t<AST, Node>
  >
  struct is_composite : std::false_type { };

  template<ast AST, ast_node_of<AST> Node, typename ...Args>
  struct is_composite<AST, Node, std::tuple<Args...>>
    : std::disjunction<has_factory<AST, Args>...> { };

  template<ast AST, ast_node_of<AST> Node>
  inline constexpr bool is_composite_v = is_composite<AST, Node>::value;

  template<
    ast AST, ast_node_of<AST> Node, size_t I = 0,
    typename T, typename ...Args
  >
  ast_factory<AST> *factory_of(std::source_location loc, T const&, Args...args) 
  {
    return factory_of<AST, Node, I + 1>(loc, args...);
  }

  template<
    ast AST, ast_node_of<AST>, size_t = 0, 
    std::derived_from<node_holder<AST>> T, typename ...Args
  >
  ast_factory<AST> *factory_of(std::source_location, T const& v, Args...) {
    return v._factory;
  }

  template<
    ast AST, ast_node_of<AST> Node, size_t I = 0,
    std::ranges::range R, typename ...Args
  >
    requires has_factory_v<AST, std::ranges::range_value_t<R>>
  ast_factory<AST> *factory_of(std::source_location loc, R const& r, Args ...) {
    black_assume(
      !empty(r), loc, 
      "argument `{}` of `{}` cannot be empty", 
      ast_node_field_name_v<AST, Node, ast_node_field_index_t<AST, Node>(I)>, 
      ast_node_name_v<AST, Node>
    );
    
    return factory_of<AST, Node>(loc, *begin(r));
  }

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  struct ast_node_fields_base { 
    ast_node_field_type_t<AST, Node, Field> const& field() const {
      constexpr size_t index = std::to_underlying(Field);
      
      Node const *node = static_cast<Node const*>(this);
      auto impl = 
        std::static_pointer_cast<ast_impl<AST, Node> const>(node->_impl);
      
      return std::get<index>(impl->data);
    }
  };

  template<
    ast AST, ast_node_of<AST> Node, typename = ast_node_field_list_t<AST, Node>
  >
  struct ast_node_named_fields { };

  template<
    ast AST, ast_node_of<AST> Node,
    ast_node_field_index_t<AST, Node> ...Fields
  >
  struct ast_node_named_fields<
    AST, Node, 
    std::tuple<
      std::integral_constant<ast_node_field_index_t<AST, Node>, Fields>...
    >
  > : ast_node_fields_base<AST, Node, Fields>...,
      ast_node_field_member_base<
        Node, AST, Node, Fields, 
        &ast_node_fields_base<AST, Node, Fields>::field
      >... 
  { 
    template<ast_node_field_index_t<AST, Node> Field>
    ast_node_field_type_t<AST, Node, Field> const& field() const {
      return ast_node_fields_base<AST, Node, Field>::field();
    }
  };

  template<ast AST, ast_node_of<AST> Node, typename ...Args>
  struct ast_node_base<AST, Node, std::tuple<Args...>>
    : node_holder<AST>, 
      ast_factory_named_member<Node, AST>,
      ast_node_named_fields<AST, Node>
  {
    using node_holder<AST>::node_holder;

    ast_node_base() = delete;

    ast_node_base(ast_node_base const&) = default;
    ast_node_base(ast_node_base &&) = default;

    ast_node_base(
      Args ...args, std::source_location loc = std::source_location::current()
    ) requires is_composite_v<AST, Node>
      : node_holder<AST>(
        factory_of<AST, Node>(loc, args...),
        factory_of<AST, Node>(loc, args...)->template allocate<Node>(
          std::move(args)...
        )
      ) { }
    
    ast_node_base &operator=(ast_node_base const&) = default;
    ast_node_base &operator=(ast_node_base &&) = default;

    template<ast>
      friend struct ast_base;
    
    template<typename, ast A, ast_node_of<A>, typename>
      friend struct ast_factory_ctor_base;
  };

  template<size_t I, black::ast::ast_node Node>
  auto get(Node n) {
    constexpr auto Field = ast_node_field_index_t<ast_of_t<Node>, Node>(I);
    return n.template field<Field>();
  } 

  template<ast AST, ast_node_of<AST> Node>
  class ast_factory_allocator_base { 
  protected:
    template<typename ...Args>
    ast_impl_ref<AST> allocate(Args ...args) {
      ast_impl<AST, Node> impl{{std::move(args)...}};

      if(auto it = _pool.find(impl); it != _pool.end())
        if(auto ptr = it->second.lock(); ptr)
          return ptr;

      auto ptr = std::make_shared<ast_impl<AST, Node> const>(std::move(impl));
      _pool[*ptr] = ptr;
      
      return ptr;
    }

  private:
    support::map<ast_impl<AST, Node>, ast_impl_weak_ref<AST>> _pool;
  };

  template<
    typename Derived,
    ast AST, ast_node_of<AST> Node, 
    typename Args = ast_node_field_types_t<AST, Node>
  >
  struct ast_factory_ctor_base { };

  template<typename Derived, ast AST, ast_node_of<AST> Node, typename ...Args>
  struct ast_factory_ctor_base<Derived, AST, Node, std::tuple<Args...>> 
    : ast_factory_allocator_base<AST, Node>
  {
    Node construct(Args ...args) {
      return Node{
        static_cast<Derived *>(this), this->allocate(std::move(args)...)
      };
    }
  };

  template<typename Derived, ast AST, ast_node_of<AST> Node>
  struct ast_factory_ctor :
    ast_factory_ctor_base<Derived, AST, Node>,
    std::conditional_t<
      is_composite_v<AST, Node>,
      std::monostate,
      ast_node_member_base<
        Derived, AST, Node, 
        &ast_factory_ctor_base<Derived, AST, Node>::construct
      >
    > { };


  template<typename Derived, ast AST, typename = ast_node_list_t<AST>>
  struct ast_factory_base { };

  template<typename Derived, ast AST, ast_node_of<AST> ...Nodes>
  struct ast_factory_base<Derived, AST, std::tuple<Nodes...>>
    : ast_factory_ctor<Derived, AST, Nodes>... 
  { 
  protected:
    template<ast_node_of<AST> Node, typename ...Args>
    auto allocate(Args&& ...args) {
      return ast_factory_ctor<Derived, AST, Node>::allocate(
        std::forward<Args>(args)...
      );
    }

    template<ast A, ast_node_of<A>, typename>
      friend struct ast_node_base;
  };

  template<ast AST>
  struct ast_factory : ast_factory_base<ast_factory<AST>, AST> { };

}

namespace black::support {
  template<ast::ast AST>
  struct match_trait<AST> {
    using cases = ast::ast_node_list_t<AST>;
    
    template<ast::ast_node_of<AST> Node>
    static std::optional<Node> downcast(AST t) {
      return t.template to<Node>();
    }
  };
}

template<black::ast::ast_node Node>
struct std::tuple_size<Node>
  : std::tuple_size<
      black::ast::ast_node_field_list_t<
        black::ast::ast_of_t<Node>, Node
      >
    > { };

template<size_t I, black::ast::ast_node Node>
struct std::tuple_element<I, Node>
  : std::tuple_element<
      I, black::ast::ast_node_field_types_t<
        black::ast::ast_of_t<Node>, Node
      >
    > { };


template<black::ast::ast T>
struct std::hash<T> {
  size_t operator()(T v) const {
    return v.hash();
  }
};

template<black::ast::ast_node T>
struct std::hash<T> {
  size_t operator()(T v) const {
    return v.hash();
  }
};

template<
  black::ast::ast AST, black::ast::ast_node_of<AST> Node
>
struct std::hash<black::ast::internal::ast_impl<AST, Node>> {
  size_t operator()(black::ast::internal::ast_impl<AST, Node> const &impl) const
  {
    return black::support::hash(impl.index, impl.data);
  }
};

#endif // BLACK_AST_CORE_HPP
