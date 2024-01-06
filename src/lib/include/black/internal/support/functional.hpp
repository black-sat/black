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

#ifndef BLACK_SUPPORT_FUNCTIONAL_HPP
#define BLACK_SUPPORT_FUNCTIONAL_HPP

#include <variant>
#include <expected>
#include <source_location>

namespace black::support::internal {

  //
  // lazy() combinator
  //
  template<typename F>
  class lazy_t 
  {
  public:
    lazy_t(F f) : _f{std::move(f)} { }

    template<typename T>
      requires std::convertible_to<std::invoke_result_t<F>, T>
    operator T() {
      return _f();
    }

  private:
    F _f;
  };

  template<typename F>
  auto lazy(F f) {
    return lazy_t<F>{std::move(f)};
  }

  //
  // unpacking() combinator
  //
  template<typename F, typename Arg, size_t ...I>
  auto do_unpack(F f, Arg arg, std::index_sequence<I...>) 
    -> decltype(std::invoke(f, arg, get<I>(arg)...))
  {
    return std::invoke(f, arg, get<I>(arg)...);
  }

  struct no_unpack_t { };

  template<size_t N, typename F, typename Arg>
  auto unpackN(F f, Arg arg) {
    if constexpr (requires{ do_unpack(f, arg, std::make_index_sequence<N>{}); })
      return do_unpack(f, arg, std::make_index_sequence<N>{});
    else if constexpr (N > 0)
      return unpackN<N - 1>(f, arg);
    else 
      return no_unpack_t{};
  }

  template<typename F, typename Arg>
  auto unpack(F f, Arg arg)
  {
    return unpackN<0>(f, arg);
  }
  
  template<typename F, typename Arg>
    requires requires { std::tuple_size<Arg>::value; }
  auto unpack(F f, Arg arg) {
    return unpackN<std::tuple_size_v<Arg>>(f, arg);
  }

  template<typename F>
  struct unpacking_t {
    template<typename Arg>
    auto operator()(Arg arg) 
      requires (!std::same_as<decltype(unpack(this->f, arg)), no_unpack_t>)
    {
      return unpack(f, arg);
    }

    F f;
  };

  template<typename F>
  auto unpacking(F f) {
    return unpacking_t<F>{f};
  }

  //
  // dispatch combinator
  //
  struct no_dispatch_t { };

  template<typename Arg, typename F, typename ...Fs>
  auto do_dispatch(Arg arg, F f, Fs ...fs) 
  {
    if constexpr (requires { std::invoke(f, arg); })
      return std::invoke(f, arg);
    else if constexpr (sizeof...(Fs) > 0)
      return do_dispatch(arg, fs...);
    else
      return no_dispatch_t{};
  }

  template<typename Arg, typename ...Fs>
  auto do_dispatch(Arg arg, std::tuple<Fs...> tuple) {
    return std::apply([&](auto ...fs) {
      return do_dispatch(arg, fs...);
    }, tuple);
  }

  template<typename ...Fs>
  struct dispatch_t 
  {
    template<typename Arg>
    auto operator()(Arg arg)
      requires 
        (!std::same_as<decltype(do_dispatch(arg, this->fs)), no_dispatch_t>)
    {
      return do_dispatch(arg, fs);
    }

    std::tuple<Fs...> fs;
  };

  template<typename ...Fs>
  auto dispatch(Fs ...fs) {
    return dispatch_t<Fs...>{{fs...}};
  }

  struct otherwise { 
    otherwise() = default;
    otherwise(auto) { }
  };

}

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

  struct missing_case_t { 
    template<typename T>
    [[noreturn]] operator T() const { black_unreachable(); }
  };

  template<typename Case, typename ...Fs>
  struct dispatch_result : std::type_identity<missing_case_t> { };
  
  template<typename Case, typename ...Fs>
  using dispatch_result_t = typename dispatch_result<Case, Fs...>::type;
  
  template<typename Case, typename ...Fs>
    requires requires {
      dispatch(unpacking(std::declval<Fs>())...)(std::declval<Case>());
    }
  struct dispatch_result<Case, Fs...> : std::type_identity<
    decltype(
      dispatch(unpacking(std::declval<Fs>())...)(std::declval<Case>())
    )
  > { };

  template<typename Cases, typename Fs>
  struct match_result { };

  template<typename Cases, typename Fs>
  using match_result_t = typename match_result<Cases, Fs>::type;

  template<
    typename Case, typename ...Cases, typename ...Fs
  >
  struct match_result<std::tuple<Case, Cases...>, std::tuple<Fs...>>
    : std::common_type<
        dispatch_result_t<Case, Fs...>, dispatch_result_t<Cases, Fs...>...
      > { };

  template<typename ...Fs>
  struct match_result<std::tuple<>, std::tuple<Fs...>>
    : std::type_identity<missing_case_t> { };

  template<typename Case, typename ...Cases, matchable M, typename ...Fs>
  match_result_t<std::tuple<Case, Cases...>, std::tuple<Fs...>>
  do_match(M m, std::source_location loc, Fs ...fs) {
    auto casted = match_trait<M>::template downcast<Case>(m);
    if(casted) {
      if constexpr (requires { dispatch(unpacking(fs)...)(*casted); })
        return dispatch(unpacking(fs)...)(*casted);
      else
        throw bad_pattern(m, loc);
    }

    if constexpr(sizeof...(Cases) > 0)
      return do_match<Cases...>(m, loc, fs...);
    else
      black_unreachable();
  }

  template<matchable M, typename Cases = typename match_trait<M>::cases>
  struct matcher_t;

  template<matchable M, typename ...Cases>
  struct matcher_t<M, std::tuple<Cases...>>
  {
    template<typename ...Fs>
    auto operator()(Fs ...fs) {
      return do_match<Cases...>(m, loc, fs...);
    }

    M m;
    std::source_location loc;
  };

  template<matchable M>
  matcher_t<M> match(
    M const& m, std::source_location loc = std::source_location::current()
  ) {
    return {m, loc};
  }

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

namespace black::support {
  using internal::lazy;
  using internal::unpacking;
  using internal::dispatch;
  using internal::otherwise;
  using internal::match;
  using internal::matchable;
}

#endif // BLACK_SUPPORT_FUNCTIONAL_HPP
