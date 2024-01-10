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

#ifndef BLACK_AST_REFLECT_DEFS_FILE
  #error "Please define the `BLACK_AST_REFLECT_DEFS_FILE` macro before" \
         "including the <black/ast/reflect> header file."
#endif

#if !__has_include(BLACK_AST_REFLECT_DEFS_FILE)
  #error "The header file " BLACK_AST_REFLECT_DEFS_FILE " given by the " \
         "`BLACK_AST_REFLECT_DEFS_FILE` macro does not exist."
#endif

#ifndef BLACK_AST_REFLECT_BASE_NAMESPACE
  #define BLACK_AST_REFLECT_BASE_NAMESPACE black
#endif

namespace BLACK_AST_REFLECT_BASE_NAMESPACE {

  #define declare_ast(NS, AST) \
    namespace NS { \
      struct AST; \
    }
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    namespace NS { \
      struct Node; \
    }
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_factory(NS, AST, Factory, Member) \
    namespace NS { \
      struct Factory; \
    }
  #include BLACK_AST_REFLECT_DEFS_FILE

}

namespace black::ast::core {
  
  #define declare_ast(NS, AST) \
    template<> \
    struct is_ast<NS::AST> : std::true_type { };
  #include BLACK_AST_REFLECT_DEFS_FILE
  
  #define declare_ast_factory(NS, AST, Factory, Member) \
    template<> \
    struct ast_factory_type<NS::AST> : std::type_identity<NS::Factory> { };
  #include BLACK_AST_REFLECT_DEFS_FILE
  
  #define declare_ast_factory(NS, AST, Factory, Member) \
    template<> \
    struct ast_factory_name<NS::AST> { \
      static constexpr std::string_view value = #Factory; \
    };
  #include BLACK_AST_REFLECT_DEFS_FILE
  
  #define declare_ast_factory(NS, AST, Factory, Member) \
    template<> \
    struct ast_factory_member_name<NS::AST> { \
      static constexpr std::string_view value = #Member; \
    };
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast(NS, AST) \
    template<> \
    struct ast_name<NS::AST> { \
      static constexpr std::string_view value = #AST; \
    };
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct is_ast_node<NS::Node> : std::true_type { };
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct is_ast_node_of<NS::Node, NS::AST> : std::true_type { };
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct ast_of<NS::Node> : std::type_identity<NS::AST> { };
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast(NS, AST) \
    template<> \
    struct ast_node_list<NS::AST> : black::ast::core::internal::tuple_cpp<0

  #define declare_ast_node(NS, AST, Node, Doc) \
      , NS::Node

  #define end_ast(NS, AST) \
    > { };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast(NS, AST) \
    template<> \
    struct ast_node_index<NS::AST> { \
      enum class type : size_t {

  #define declare_ast_node(NS, AST, Node, Doc) \
        Node,

  #define end_ast(NS, AST) \
      }; \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct ast_node_index_of<NS::AST, NS::Node> \
      : std::integral_constant< \
          ast_node_index_t<NS::AST>, ast_node_index_t<NS::AST>::Node \
        > { };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct ast_node_name<NS::AST, NS::Node> { \
      static constexpr std::string_view value = #Node; \
    };
  #include BLACK_AST_REFLECT_DEFS_FILE
  
  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct ast_node_doc<NS::AST, NS::Node> { \
      static constexpr std::string_view value = #Doc; \
    };
  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct ast_node_field_index<NS::AST, NS::Node> { \
      enum class type : size_t {

  #define declare_field(NS, AST, Node, Field, Type, Doc) \
        Field,

  #define end_ast_node(NS, AST, Node) \
      }; \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE


  #define declare_ast_node(NS, AST, Node, Doc) \
    template<> \
    struct ast_node_field_list<NS::AST, NS::Node> \
      : black::ast::core::internal::tuple_cpp<0

  #define declare_field(NS, AST, Node, Field, Type, Doc) \
      , std::integral_constant< \
          ast_node_field_index_t<NS::AST, NS::Node>, \
          ast_node_field_index_t<NS::AST, NS::Node>::Field \
        >
  
  #define end_ast_node(NS, AST, Node) \
    > { };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_field(NS, AST, Node, Field, Type, Doc) \
    template<> \
    struct ast_node_field_type< \
      NS::AST, NS::Node, ast_node_field_index_t<NS::AST, NS::Node>::Field \
    > : std::type_identity<Type> { };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_field(NS, AST, Node, Field, Type, Doc) \
    template<> \
    struct ast_node_field_name< \
      NS::AST, NS::Node, ast_node_field_index_t<NS::AST, NS::Node>::Field \
    > { \
      static constexpr std::string_view value = #Field; \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_field(NS, AST, Node, Field, Type, Doc) \
    template<> \
    struct ast_node_field_doc< \
      NS::AST, NS::Node, ast_node_field_index_t<NS::AST, NS::Node>::Field \
    > { \
      static constexpr std::string_view value = #Doc; \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    template< \
      typename Derived, \
      typename Base, typename R, typename ...Args, \
      R (Base::*Ptr)(Args...) \
    > \
    struct ast_node_member_base<Derived, NS::AST, NS::Node, Ptr> { \
      R Node(Args const& ...args) { \
        return (static_cast<Derived *>(this)->*Ptr)(args...); \
      } \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_field(NS, AST, Node, Field, Type, Doc) \
    template< \
      typename Derived, \
      typename Base, typename R, \
      R (Base::*Ptr)() const \
    > \
    struct ast_node_field_member_base< \
      Derived, \
      NS::AST, NS::Node, ast_node_field_index_t<NS::AST, NS::Node>::Field, Ptr \
    > { \
      R Field() const { \
        return (static_cast<Derived const*>(this)->*Ptr)(); \
      } \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE

} // namespace black::ast::core

namespace BLACK_AST_REFLECT_BASE_NAMESPACE {

  #define declare_ast_factory(NS, AST, Factory, Member) \
    namespace NS { \
      struct Factory; \
    }

  #include BLACK_AST_REFLECT_DEFS_FILE

}

namespace black {

  #define declare_ast_factory(NS, AST, Factory, Member) \
    template<typename Derived> \
    struct ast::core::ast_factory_named_member<Derived, NS::AST> { \
      NS::Factory *Member() const { \
        return static_cast<NS::Factory *>( \
          static_cast<Derived const*>(this)->factory() \
        ); \
      } \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE

}

namespace BLACK_AST_REFLECT_BASE_NAMESPACE {

  #define declare_ast(NS, AST) \
    struct NS::AST : ast::core::internal::ast_base<NS::AST> { \
      using ast::core::internal::ast_base<NS::AST>::ast_base; \
  };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_node(NS, AST, Node, Doc) \
    struct NS::Node : ast::core::internal::ast_node_base<NS::AST, NS::Node> { \
      using \
        ast::core::internal::ast_node_base<NS::AST, NS::Node>::ast_node_base; \
    };

  #include BLACK_AST_REFLECT_DEFS_FILE

  #define declare_ast_factory(NS, AST, Factory, Member) \
    struct NS::Factory : ast::core::internal::ast_factory<NS::AST> { };

  #include BLACK_AST_REFLECT_DEFS_FILE

}
