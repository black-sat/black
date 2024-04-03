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

#include <black/support>

#include <string_view>
#include <source_location>
#include <tuple>

//
// Public interface for reflection of AST nodes.
//
// See reflect.hpp to see how these are implemented from the expansion 
// of defs.hpp
//
namespace black::ast::core {

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

  template<typename T>
  concept ast_type = ast<T> || ast_node<T>;

  template<typename T, typename AST>
  concept ast_type_of = std::same_as<T, AST> || ast_node_of<T, AST>;

  template<ast AST>
  struct ast_name { };

  template<ast AST>
  inline constexpr auto ast_name_v = ast_name<AST>::value;

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

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_doc { };
  
  template<ast AST, ast_node_of<AST> Node>
  inline constexpr auto ast_node_doc_v = ast_node_doc<AST, Node>::value;

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
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  struct ast_node_field_doc { };
  
  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  inline constexpr auto ast_node_field_doc_v = 
    ast_node_field_doc<AST, Node, Field>::value;

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

  namespace internal {
    template<ast AST>
    class node_holder;
  }

  template<typename AST>
  struct ast_custom_members { };

  template<typename AST>
  struct ast_custom_ctor { }; 
  
  template<typename Node>
  struct ast_node_custom_members { };

  template<typename Node>
  struct ast_node_custom_ctor { };
}

//
// Implementation of AST nodes
//
namespace black::ast::core::internal {

  template<int Dummy, typename ...Args>
  struct tuple_cpp : std::type_identity<std::tuple<Args...>> { };

  struct ast_metadata {
    const size_t hash;
  };

  template<ast AST>
  struct ast_impl_base {
    const ast_node_index_t<AST> index;
    const ast_metadata metadata;

    bool operator==(ast_impl_base const&) const = default;
  };

  template<ast AST>
  using ast_impl_ref = std::shared_ptr<ast_impl_base<AST> const>;
  
  template<ast AST>
  using ast_impl_weak_ref = std::weak_ptr<ast_impl_base<AST> const>;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_impl : ast_impl_base<AST> {
    
    ast_impl(ast_node_field_types_t<AST, Node> _data, ast_metadata meta) 
      : ast_impl_base<AST>{ast_node_index_of_v<AST, Node>, meta}, 
        data{_data} { }
    
    bool operator==(ast_impl const&) const = default;

    const ast_node_field_types_t<AST, Node> data;
  };

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
      using index_t = ast_node_index_t<AST>;
    
      unique_id_t unique_id() const { 
        return unique_id_t(std::bit_cast<uintptr_t>(_impl.get()));
      }
      
      size_t hash() const {
        return impl()->metadata.hash;
      }

      index_t index() const { return _impl->index; }

      template<ast_node_of<AST> Node, typename ...Args>
      static ast_impl_ref<AST> allocate(Args ...args) {
        ast_node_field_types_t<AST, Node> data{args...};
        return std::make_shared<ast_impl<AST, Node>>(
          std::move(data), ast_metadata{support::hash(data)}
        );
      }

    protected:
      ast_impl_ref<AST> impl() const { return _impl; }

      explicit node_holder(ast_impl_ref<AST> i) : _impl{i} { }

      template<ast A, ast_node_of<A> N, ast_node_field_index_t<A, N>>
      friend struct ast_node_fields_base;

      template<ast_node Node, ast A>
        requires ast_node_of<Node, A>
      friend std::optional<Node> cast(node_holder<A> const& h);
      
      template<ast_node Node, ast A>
        requires ast_node_of<Node, A>
      friend std::optional<Node> cast(node_holder<A>&& h);

      ast_impl_ref<AST> _impl;
  };

  template<ast_node Node, ast AST>
    requires ast_node_of<Node, AST>
  std::optional<Node> cast(node_holder<AST> const& h) {
    if(h._impl->index == ast_node_index_of_v<AST, Node>)
      return Node{h._impl};
    return {};
  }
  
  template<ast_node Node, ast AST>
    requires ast_node_of<Node, AST>
  std::optional<Node> cast(node_holder<AST>&& h) {
    if(h._impl->index == ast_node_index_of_v<AST, Node>)
      return Node{std::move(h._impl)};
    return {};
  }

  template<
    ast AST, ast_node_of<AST> Node, typename = ast_node_field_types_t<AST, Node>
  >
  struct ast_node_base;

  template<ast AST>
  struct ast_base : node_holder<AST>, ast_custom_members<AST>
  {
    using node_holder<AST>::node_holder;

    ast_base() = delete;
    ast_base(ast_base const&) = default;
    ast_base(ast_base &&) = default;
    
    template<ast_node_of<AST> Node>
    ast_base(Node n) 
      : node_holder<AST>(n._impl) { }

    template<typename ...Args>
      requires requires(Args ...args) { 
        ast_custom_ctor<AST>::convert(std::move(args)...);
      }
    ast_base(Args ...args) 
      : node_holder<AST>(
          ast_custom_ctor<AST>::convert(std::move(args)...).impl()
        ) { }

    template<typename ...Args>
      requires requires(Args ...args) { 
        ast_custom_ctor<AST>::init(std::move(args)...); 
      }
    explicit ast_base(Args ...args) 
      : node_holder<AST>(
          ast_custom_ctor<AST>::init(std::move(args)...).impl()
        ) { }

    ast_base &operator=(ast_base const&) = default;
    ast_base &operator=(ast_base &&) = default;

    template<ast A, ast_node_of<A>, typename>
      friend struct ast_node_base;
  };
  
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
      ast_node_named_fields<AST, Node>,
      ast_node_custom_members<Node>
  {
    using node_holder<AST>::node_holder;

    ast_node_base(ast_node_base const&) = default;
    ast_node_base(ast_node_base &&) = default;

    explicit ast_node_base(Args ...args)
      : node_holder<AST>{node_holder<AST>::template allocate<Node>(args...)} { }
    
    template<typename ...Ts>
      requires requires(Ts ...args) { 
        ast_node_custom_ctor<Node>::convert(std::move(args)...);
      }
    ast_node_base(Ts ...args) 
      : node_holder<AST>(
          ast_node_custom_ctor<Node>::convert(std::move(args)...).impl()
        ) { }
    
    template<typename ...Ts>
      requires requires(Ts ...args) { 
        ast_node_custom_ctor<Node>::init(std::move(args)...);
      }
    explicit ast_node_base(Ts ...args) 
      : node_holder<AST>(
          ast_node_custom_ctor<Node>::init(std::move(args)...).impl()
        ) { }

    ast_node_base &operator=(ast_node_base const&) = default;
    ast_node_base &operator=(ast_node_base &&) = default;

    template<ast>
      friend struct ast_base;
  };

  template<size_t I, ast_node Node>
  auto get(Node n) {
    constexpr auto Field = ast_node_field_index_t<ast_of_t<Node>, Node>(I);
    return n.template field<Field>();
  }
} // black::ast::core::internal

namespace black::ast::core {

  using internal::cast;

  template<typename From, typename To>
  concept castable_to = ast_node<To> && requires(From f) { 
    { cast<To>(f) } -> std::same_as<std::optional<To>>;
  };

  template<ast_node Node, castable_to<Node> T>
  std::optional<Node> cast(std::optional<T> o) {
    if(o)
      return cast<Node>(*o);
    return {};
  }

  template<ast_node Node, castable_to<Node> T, typename E>
  std::expected<Node, E> cast(std::expected<T, E> const& e) {
    if(e)
      if(auto casted = cast<Node>(*e); casted)
        return *casted;
    return std::unexpected(e.error());
  }
}

namespace black::support {

  template<ast::core::ast_node Node1, ast::core::ast_node Node2>
    requires (
      !std::same_as<Node1, Node2> &&
      std::same_as<
        ast::core::ast_of_t<Node1>, ast::core::ast_of_t<Node2>
      >
    )
  struct common_result<Node1, Node2> 
    : std::type_identity<ast::core::ast_of_t<Node1>> { };

  template<ast::core::ast AST>
  struct match_cases<AST> : ast::core::ast_node_list<AST> { };
  
  template<ast::core::ast_node Node>
  struct match_cases<Node> : std::type_identity<std::tuple<Node>> { };
  
  template<typename T, ast::core::ast_node Node>
    requires ast::core::castable_to<T, Node>
  struct match_downcast<T, Node> {
    static std::optional<Node> downcast(T const& t) {
      return ast::core::cast<Node>(t);
    }
    
    static std::optional<Node> downcast(T&& t) {
      return ast::core::cast<Node>(std::move(t));
    }
  };
  
  template<ast::core::ast_node Node>
  struct match_downcast<Node, Node> {
    static std::optional<Node> downcast(Node t) {
      return std::move(t);
    }
  };
  
  template<ast::core::ast_node NodeFrom, ast::core::ast_node NodeTo>
    requires (!std::same_as<NodeFrom, NodeTo>)
  struct match_downcast<NodeFrom, NodeTo> {
    static std::optional<NodeTo> downcast(NodeFrom) {
      return {};
    }
  };
}

template<black::ast::core::ast_node Node>
struct std::tuple_size<Node>
  : std::tuple_size<
      black::ast::core::ast_node_field_list_t<
        black::ast::core::ast_of_t<Node>, Node
      >
    > { };

template<size_t I, black::ast::core::ast_node Node>
struct std::tuple_element<I, Node>
  : std::tuple_element<
      I, black::ast::core::ast_node_field_types_t<
        black::ast::core::ast_of_t<Node>, Node
      >
    > { };

#endif // BLACK_AST_CORE_HPP
