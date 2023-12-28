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

//
// Here we declare the infrastructure for pattern matching. The machinery is
// based on three elements which have to be provided by matchable types:
// - a specialization of match_cases<T> returning a tuple of cases types
// - a T::to<U>() function that performs the downcast
// - a T::is<U>() function that tells if the downcast is possible

namespace black::support {
  template<typename T>
  struct match_cases { };

  template<typename T>
  using match_cases_t = typename match_cases<T>::type;
}

namespace black::support::internal {

  template<typename T>
  concept matchable = requires (T t) {
    typename match_cases<T>::type;
    
    { std::tuple_size<match_cases_t<T>>::value };
    
    requires (std::tuple_size<match_cases_t<T>>::value > 0);
    
    t.template to<typename std::tuple_element<0, match_cases_t<T>>::type>();

    t.template is<typename std::tuple_element<0, match_cases_t<T>>::type>();
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
  // trick. This is generic and works for any type that expose the `to<>` and
  // `is<>` functions. The `H` parameter is the main class from which one wants
  // to match (e.g. `term`). The `Cases` parameter is a tuple of types to try to
  // match the matched object to. The default value for `Cases` is the correct
  // one to use in the common case of `H` being a type that satisfies the
  // interface for matchable types.
  //
  template<typename H, typename Cases = match_cases_t<H>>
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
      if(h.template is<Case>())
        return dispatch(*h.template to<Case>(), loc, handlers...);
      
      return dispatcher<H, std::tuple<Cases...>>::match(
        h, loc, handlers...
      );
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
      black_assert(h.template is<Case>());
      
      return dispatch(*h.template to<Case>(), loc, handlers...);
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
    template<typename T>
    otherwise(T const&) { }
  };

  //
  // Subclass of `std::variant` that supports the pattern matching interface
  //
  template<typename ...Cases>
  struct either : private std::variant<Cases...> 
  {
    using std::variant<Cases...>::variant;

    bool operator==(either const&) const = default;

    template<typename T>
      requires (std::is_same_v<T, Cases> || ...)
    std::optional<T> to() const {
      if(std::holds_alternative<T>(*this))
        return std::get<T>(*this);
      return {};
    }

    template<typename T>
    bool is() const {
      return to<T>().has_value();
    }  

  };

}

namespace black::support {
  using internal::match;
  using internal::matchable;
  using internal::otherwise;
  using internal::either;
}

template<typename ...Cases>
  struct black::support::match_cases<black::support::either<Cases...>> 
    : std::type_identity<std::tuple<Cases...>> { };

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

template<typename ...Cases>
struct std::hash<::black::support::either<Cases...>> {
  size_t operator()(::black::support::either<Cases...> const& v) const {
    return match(v)(
      [](auto x) {
        return ::black::support::hash(x);
      }
    );
  }
};

#endif // BLACK_SUPPORT_MATCH_HPP
