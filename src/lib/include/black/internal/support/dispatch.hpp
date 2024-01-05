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

#ifndef BLACK_SUPPORT_DISPATCH_HPP
#define BLACK_SUPPORT_DISPATCH_HPP

#include <source_location>
#include <utility>
#include <tuple>

namespace black::support {

  template<typename T>
  struct unpack_size { };

  template<typename T>
    requires requires { std::tuple_size<T>::value; }
  struct unpack_size<T> 
    : std::integral_constant<size_t, std::tuple_size<T>::value + 1> { };

  template<typename T>
  inline constexpr size_t unpack_size_v = unpack_size<T>::value;

  template<size_t I, typename T>
  struct unpack_element : std::tuple_element<I - 1, T> { };

  template<typename T>
  struct unpack_element<0, T> : std::type_identity<T> { };

  template<size_t I, typename T>
  using unpack_element_t = typename unpack_element<I, T>::type;

  template<size_t I, typename T>
    requires (I == 0 || requires(T v) { get<I - 1>(v); })
  decltype(auto) unpack(T&& v) {
    if constexpr(I == 0)
      return std::forward<T>(v);
    else
      return get<I - 1>(std::forward<T>(v));
  }
}


namespace black::support::internal {

  template<typename Callable, typename Arg>
  struct is_invocable_unpacked : std::false_type { };

  template<typename Callable, typename Arg>
  inline constexpr bool is_invocable_unpacked_v = 
    is_invocable_unpacked<Callable, Arg>::value;

  template<
    typename Callable, typename Arg, 
    typename Indexes = std::make_index_sequence<unpack_size_v<Arg>>
  >
  struct is_invocable_unpacked_ : std::false_type { };

  template<typename Callable, typename Arg, size_t ...I>
  struct is_invocable_unpacked_<Callable, Arg, std::index_sequence<I...>>
    : std::is_invocable<Callable, unpack_element_t<I, Arg>...> { };

  template<typename Callable, typename Arg>
    requires requires { unpack_size<Arg>::value; }
  struct is_invocable_unpacked<Callable, Arg>
    : is_invocable_unpacked_<Callable, Arg> { };

  template<typename Callable, typename Arg, size_t ...I>
  auto invoke_unpacked(
    Callable const& callable, Arg&& arg, std::index_sequence<I...>
  ) {
    return std::invoke(callable, unpack<I>(std::forward<Arg>(arg))...);
  }

  struct missing_case_t {
    template<typename T>
    [[noreturn]] operator T() const { black_unreachable(); }
  };

  template<typename Arg>
  missing_case_t dispatch_impl(std::source_location loc, Arg&& arg) {
    throw bad_pattern(std::forward<Arg>(arg), loc);
  }

  template<typename Arg, typename Callable, typename ...Callables>
  auto dispatch_impl(
    std::source_location loc,
    Arg&& arg, Callable callable, Callables ...callables
  ) {
    using TArg = std::remove_cvref_t<Arg>;
    if constexpr(is_invocable_unpacked_v<Callable, TArg>) 
      return invoke_unpacked(
        callable, std::forward<Arg>(arg),
        std::make_index_sequence<unpack_size_v<TArg>>{}
      );
    else if constexpr(std::is_invocable_v<Callable, Arg>)
      return std::invoke(callable, std::forward<Arg>(arg));
    else
      return dispatch_impl(loc, std::forward<Arg>(arg), callables...);
  }

  template<typename Arg>
  class dispatch_t 
  {
  public:
    dispatch_t(Arg const& arg, std::source_location loc) 
      : _arg{arg}, _loc{loc} { }

    template<typename ...Callables>
    auto operator()(Callables ...callables) const {
      return dispatch_impl(_loc, _arg, callables...);
    }

  private:
    Arg const& _arg;
    std::source_location _loc;
  };

  struct otherwise {
    otherwise() = default;

    template<typename T>
    otherwise(T const&) { }
  };

  template<typename Arg>
  auto dispatch(
    Arg const& arg, std::source_location loc = std::source_location::current()
  ) {
    return dispatch_t<Arg>{arg, loc};
  }
}

namespace black::support {
  using internal::dispatch;
  using internal::otherwise;
}

#endif // BLACK_SUPPORT_DISPATCH_HPP
