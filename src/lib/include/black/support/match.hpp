//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#ifndef BLACK_SUPPORT_MATCH_HPP
#define BLACK_SUPPORT_MATCH_HPP


namespace black::support::internal {

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
      std::make_index_sequence<std::tuple_size<T>::value>{}
    )
  ) {
    return unpack(
      std::forward<Handler>(handler), h, 
      std::make_index_sequence<std::tuple_size<T>::value>{}
    );
  }

  //
  // This concept holds if the call to `unpack` is well-formed
  //
  template<typename H, typename Handler>
  concept unpackable = requires(Handler handler, H h) {
    unpack(handler, h);    
  };

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
    if constexpr(unpackable<T, Handler>) 
      return unpack(std::forward<Handler>(handler), obj);
    else if constexpr(std::is_invocable_v<Handler, T>)
      return std::invoke(std::forward<Handler>(handler), obj);
    else 
      return dispatch(obj, std::forward<Handlers>(handlers)...);
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
  // Now the matcher class itself. The `H` paremeter is the main class from
  // which one wants to match (e.g. `formula<Syntax>`). The `Cases` parameter is
  // a tuple of types to try to match the matched object to. The default value
  // for `Cases` is the correct one to use in the common case of `H` being a
  // hierarchy type.
  //
  template<typename H, typename Cases>
  struct matcher;

  template<typename H, typename Case, typename ...Cases>
    //requires can_cast_to<H, Case>
  struct matcher<H, std::tuple<Case, Cases...>>
  {
    //
    // The return type of `match()` is computed with `std::common_type`, which
    // has been specialized for hierarchies above. Again, it is cumbersome to
    // repeat the body twice but there's no other way.
    //
    template<typename ...Handlers>
    static auto match(H h, Handlers&& ...handlers) 
      -> std::common_type_t<
        decltype(dispatch(
          *h.template to<Case>(), std::forward<Handlers>(handlers)...
        )),
        decltype(matcher<H, std::tuple<Cases...>>::match(
          h, std::forward<Handlers>(handlers)...
        ))
      >
    {
      if(h.template is<Case>())
        return dispatch(
          *h.template to<Case>(), std::forward<Handlers>(handlers)...
        );
      
      return matcher<H, std::tuple<Cases...>>::match(
        h, std::forward<Handlers>(handlers)...
      );
    }
  };

  //
  // The base case of the recursion above is the singleton list of cases. If we
  // reach this, and the main type cannot be casted to this last case, it means
  // it could not have been casted to any of the elements included in the list
  // of cases, so we raise an exception.
  //
  template<typename H, typename Case>
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
      throw pattern_error();
    }
  };

  //
  // Little catch-all type used as a wildcard in patter matching 
  //
  struct otherwise {
    template<typename T>
    otherwise(T const&) { }
  };

  //
  // Utility type to declare a simple union type from a list of alternatives
  //
  template<typename ...Cases>
  struct union_type : private std::variant<Cases...> {
    using std::variant<Cases...>::variant;

    bool operator==(union_type const&) const = default;

    static constexpr bool is_union_type = true;

    template<typename T>
    std::optional<T> to() const {
      if(std::holds_alternative<T>(*this))
        return std::get<T>(*this);
      return {};
    }

    template<typename T>
    bool is() const {
      return to<T>().has_value();
    }

    template<typename ...Handlers>
    auto match(Handlers ...h) const {
      return matcher<
        union_type<Cases...>, std::tuple<Cases...>
      >::match(*this, h...);
    }
  };

}

namespace std {
  template<typename T>
    requires requires { T::is_union_type; }
  struct hash<T> {
    size_t operator()(T const& v) {
      return v.match(
        []<typename U>(U x) {
          return std::hash<U>{}(x);
        }
      );
    }
  };
}

#define black_union_type(T1, ...) \
  public black::support::union_type<T1, __VA_ARGS__> { \
    using black::support::union_type<T1, __VA_ARGS__>::union_type; \
  }

namespace black::support {
  using internal::matcher;
  using internal::otherwise;
  using internal::union_type;
}

#endif // BLACK_SUPPORT_MATCH_HPP
