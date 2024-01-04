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

#include <variant>
#include <expected>

//
// Here we declare the infrastructure for pattern matching. The machinery is
// based on a specialization of `match_trait<T>` which has to be provided by
// any matchable type `T`.
//
// The specialization must provide:
// - a `cases` alias to a tuple of alternative types for the match
// - a `downcast<U>(T)` function that downcasts the `T` to an `std::optional<U>`
//   if possible, or std::nullopt otherwise (only when `U` is among `cases`)
//

namespace black::support {
  template<typename T>
  struct match_trait { };
}

namespace black::support::internal {

  template<typename T>
  concept matchable = requires (T t) {
    
    requires (std::tuple_size_v<typename match_trait<T>::cases> > 0);
    
    match_trait<T>::template downcast<
      typename std::tuple_element<0, typename match_trait<T>::cases>::type
    >(t);
  };


  //
  // The first thing we need is a function to do this std::apply-like unpacking
  // of the hierarchy to the called lambda. 
  // If the argument is a pointer, we unpack the pointee.
  //
  template<
    typename Handler, typename T, typename Base = std::remove_pointer_t<T>,
    size_t ...I
  >
    requires std::invocable<Handler, T, std::tuple_element_t<I, Base>...>
  auto unpack(Handler&& handler, T h, std::index_sequence<I...>) {
    if constexpr(std::is_pointer_v<T>)
      return std::invoke(std::forward<Handler>(handler), h, get<I>(*h)...);
    else
      return std::invoke(std::forward<Handler>(handler), h, get<I>(h)...);
  }

  //
  // Entry function to unpack the index sequence.
  // If the argument is a pointer, we unpack the pointee.
  //
  template<
    typename Handler, typename T, typename Base = std::remove_pointer_t<T>
  >
  auto unpack(Handler&& handler, T h)
  -> decltype(
    unpack(
      std::forward<Handler>(handler), h, 
      std::make_index_sequence<std::tuple_size<Base>::value>{}
    )
  ) {
    return unpack(
      std::forward<Handler>(handler), h, 
      std::make_index_sequence<std::tuple_size<Base>::value>{}
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
  // This is a dummy return type for missing cases in partial pattern matches.
  // A conversion operator is available for any type, but it is never actually 
  // called. See also the `std::common_type` specialization below.
  //
  struct missing_case_t {
    template<typename T>
    [[noreturn]] operator T() const { black_unreachable(); }
  };

  //
  // The `dispatch()` function takes a matchable object and the list of handlers
  // and calls the first handler that can be called either directly or by
  // unpacking. Note that we check the unpacked case before to give it priority,
  // so that e.g. a lambda such as `[](conjunction, auto ...args) { }` picks up
  // the unpacked children in `args`.
  //
  // The single-argument version is only called if no handler matches the type 
  // of the object, so we raise a `bad_pattern` exception.
  //
  template<typename T>
  missing_case_t dispatch(T const& obj, std::source_location const& loc) { 
    throw bad_pattern(obj, loc);
  }
  
  template<typename T, typename Handler, typename ... Handlers>
  auto dispatch(
    T const& obj, std::source_location const& loc, 
    Handler const& handler, Handlers const& ...handlers
  ) {
    if constexpr(unpackable<T, Handler>) 
      return unpack(handler, obj);
    else if constexpr(std::is_invocable_v<Handler, T>)
      return std::invoke(handler, obj);
    else 
      return dispatch(obj, loc, handlers...);
  }

  //
  // Little helper to compute the return type of the `dispatch` function above 
  //
  template<typename T, typename ...Handlers>
  using dispatch_return_t = decltype(
    dispatch(
      std::declval<T>(), std::declval<std::source_location>(), 
      std::declval<Handlers>()...
    )
  );

  //
  // Finally, the `dispatcher` class, which calls the machinery above to do the
  // trick. This is generic and works for any `matchable` type as defined above.
  // The `H` parameter is the main class from which one wants to match (e.g.
  // `term`).
  //
  template<typename H, typename Cases = match_trait<H>::cases>
  struct dispatcher;

  template<typename H, typename Case, typename ...Cases>
  struct dispatcher<H, std::tuple<Case, Cases...>>
  {
    //
    // The return type of `match()` is computed with `std::common_type`, which
    // can also be specialized for custom types.
    //
    template<typename ...Handlers>
    using dispatch_common_t = std::common_type_t<
      dispatch_return_t<Case, Handlers...>,
      dispatch_return_t<Cases, Handlers...>...
    >;

    template<typename ...Handlers>
    static dispatch_common_t<Handlers...> 
    match(
      H const& h, std::source_location const& loc, Handlers const& ...handlers
    ) {
      std::optional<Case> casted = match_trait<H>::template downcast<Case>(h);
      if(casted)
        return dispatch(*casted, loc, handlers...);
      
      return dispatcher<H, std::tuple<Cases...>>::match(h, loc, handlers...);
    }
  };

  //
  // Base case of the recursion, when we only have one case.
  //
  template<typename H, typename Case>
  struct dispatcher<H, std::tuple<Case>> 
  { 
    template<typename ...Handlers>
    static dispatch_return_t<Case, Handlers...> 
    match(
      H const& h, std::source_location const& loc, Handlers const& ...handlers
    ) {
      std::optional<Case> casted = match_trait<H>::template downcast<Case>(h);
      black_assert(casted);
      
      return dispatch(*casted, loc, handlers...);
    }
  };

  template<matchable H>
  struct matcher {

    template<typename ...Handlers>
    auto operator()(Handlers const& ...handlers) const {
      return dispatcher<H>::match(_h, _loc, handlers...);
    }

    H const& _h;
    std::source_location _loc;
  };

  template<matchable H>
  matcher<H> match(
    H const& h, std::source_location loc = std::source_location::current()
  ) {
    return {h, loc};
  }

  //
  // Little catch-all type used as a wildcard in patter matching 
  //
  struct otherwise {
    otherwise() = default;

    template<typename T>
    otherwise(T const&) { }
  };

}

//
// Specialization of `match_trait` for standard types `std::variant`, and
// `std::expected`.
//
namespace black::support {

  template<typename ...Cases>
  struct match_trait<std::variant<Cases...>> {
    using cases = std::tuple<Cases...>;
    
    template<typename U>
      requires (std::is_same_v<U, Cases> || ...)
    static std::optional<U> downcast(std::variant<Cases...> const& v) {
      if(std::holds_alternative<U>(v))
        return {std::get<U>(v)};
      return {};
    }
  };
  
  template<typename T, typename E>
  struct match_trait<std::expected<T, E>> {
    using cases = std::tuple<T, E>;
    
    template<typename U>
      requires (std::is_same_v<U, T>)
    static std::optional<U> downcast(std::expected<T, E> const& v) {
      if(v.has_value())
        return {v.value()};
      return {};
    }
    
    template<typename U>
      requires (std::is_same_v<U, E>)
    static std::optional<U> downcast(std::expected<T, E> const& v) {
      if(!v.has_value())
        return {v.error()};
      return {};
    }
    
  };
  
  template<typename T>
  struct match_trait<std::optional<T>> {
    using cases = std::tuple<T, internal::otherwise>;
    
    template<typename U>
      requires (std::is_same_v<U, T>)
    static std::optional<U> downcast(std::optional<T> const& v) {
      if(v.has_value())
        return {v.value()};
      return {};
    }
    
    template<typename U>
      requires (std::is_same_v<U, internal::otherwise>)
    static std::optional<U> downcast(std::optional<T> const& v) {
      if(!v.has_value())
        return {internal::otherwise{}};
      return {};
    }
    
  };

}


template<>
struct std::common_type<
  ::black::support::internal::missing_case_t, 
  ::black::support::internal::missing_case_t
> : std::type_identity<::black::support::internal::missing_case_t> { };

template<typename T>
struct std::common_type<::black::support::internal::missing_case_t, T> : 
  std::type_identity<T> { };

template<typename T>
struct std::common_type<T, ::black::support::internal::missing_case_t> : 
  std::type_identity<T> { };

namespace black::support {
  using internal::match;
  using internal::matchable;
  using internal::otherwise;
}

#endif // BLACK_SUPPORT_MATCH_HPP
