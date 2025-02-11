//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2025 Nicola Gigante
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

#ifndef BLACK_PARSING_FUNCTIONAL_HPP
#define BLACK_PARSING_FUNCTIONAL_HPP

#include <ranges>
#include <print>

namespace black::parsing {

  template<std::ranges::range R, typename T = std::ranges::range_value_t<R>>
  parser<T[]> yield(R rng) {
    return [rng = std::move(rng)] -> parsed<T[]> {
      for(auto v : rng)
        co_yield std::move(v);
    };
  }

  template<typename T>
  parser<T[]> single(T v) {
    return [v = std::move(v)] -> parsed<T[]> {
      co_yield std::move(v);
    };
  }

  template<typename T>
  parser<T[]> single(parser<T> p) {
    return [=] -> parsed<T[]> {
      co_yield co_await p;
    };
  }

  template<typename T>
  parser<T[]> empty() {
    return [] -> parsed<T[]> { co_return; };
  }

  template<
    typename T, 
    std::invocable<T> F, 
    typename R = std::invoke_result_t<F, T>
  >
  parser<R> apply(parser<T> p, F f) {
    return [=] -> parsed<R> {
      co_return f(co_await p);
    };
  }
  
  template<
    typename T,
    typename F,
    typename R = std::ranges::range_value_t<
      std::invoke_result_t<F, std::vector<T>>
    >
  >
  parser<R[]> apply(parser<T[]> p, F f) {
    return [=] -> parsed<R[]> {
      for(auto v : f(co_await p))
        co_yield std::move(v);
    };
  }

  template<typename T>
  parser<T[]> sequence(parser<parser<T>[]> ps) {
    return [=] -> parsed<T[]> {
      for(auto p : co_await ps)
        co_yield co_await p;
    };
  }

  template<
    typename T,
    std::invocable<T> F, 
    typename R = std::invoke_result_t<F, T>
  >
  parser<R[]> transform(parser<T[]> p, F f) {
    return apply(p, std::views::transform(f));
  }

  template<typename T, predicate_for<T> P>
  parser<T[]> filter(parser<T[]> p, P pred) {
    return apply(p, std::views::filter(pred));
  }

  template<typename T>
  parser<T[]> reversed(parser<T[]> p) {
    return apply(p, std::views::reverse);
  }

  template<typename T>
  parser<T[]> take(parser<T[]> p, size_t n) {
    return apply(p, std::views::take(n));
  }

  template<typename T, predicate_for<T> P>
  parser<T[]> take_while(parser<T[]> p, P pred) {
    return apply(p, std::views::take_while(pred));
  }

  template<typename T>
  parser<T[]> drop(parser<T[]> p, size_t n) {
    return apply(p, std::views::drop(n));
  }

  template<typename T, predicate_for<T> P>
  parser<T[]> drop_while(parser<T[]> p, P pred) {
    return apply(p, std::views::drop_while(pred));
  }
  
  template<typename T>
  parser<T[]> concat(parser<T[]> p1, parser<T[]> p2) {
    return [=] -> parsed<T[]> {
      for(auto v : co_await p1)
        co_yield std::move(v);
      for(auto v : co_await p2)
        co_yield std::move(v);
    };
  }
  
  template<typename T, std::same_as<T> ...Ts>
  parser<T[]> concat(parser<T[]> p, parser<Ts[]> ...ps) {
    return [=] -> parsed<T[]> {
      parser<T[]> all[] = { p, ps... };
      
      for(auto q : all)
        for(auto v : co_await q)
          co_yield std::move(v);
    };
  }

  template<template<typename ...> class C = std::vector, typename T>
  parser<C<T>> collect(parser<T[]> p) {
    return [=] -> parsed<C<T>> {
      co_return co_await p | std::ranges::to<C>();
    };
  }

  template<typename T, typename S>
  parser<T[]> sep_some(parser<T> p, parser<S> s) {
    return concat(single(p), many(s + p));
  }
  
  template<typename T, typename S>
  parser<T[]> sep_many(parser<T> p, parser<S> s) {
    return sep_some(p, s) | empty<T>();
  }

  template<typename ...Ts, typename R = std::tuple<Ts...>>
  parser<R[]> zip(parser<Ts[]> ...ps) {
    return [=] -> parsed<R[]> {
      for(auto t : std::views::zip(co_await ps...))
        co_yield std::move(t);
    };
  }

  template<
    size_t N, 
    typename ...Ts, 
    typename R = std::tuple_element_t<N, std::tuple<Ts...>>
  >
  parser<R> index(parser<std::tuple<Ts...>> p) {
    return apply(p, [](auto t) { return std::move(std::get<N>(t)); });
  }

  template<
    size_t N, 
    typename ...Ts, 
    typename R = std::tuple_element_t<N, std::tuple<Ts...>>
  >
  parser<R[]> index(parser<std::tuple<Ts...>[]> p) {
    return [=] -> parsed<R[]> {
      for(auto t : co_await p)
        co_yield std::move(std::get<N>(t));
    };
  }
  

  template<
    size_t N, 
    typename T1, typename T2,
    typename R = std::tuple_element_t<N, std::tuple<T1, T2>>
  >
  parser<R> index(parser<std::pair<T1, T2>> p) {
    return apply(p, [](auto t) { return std::move(std::get<N>(t)); });
  }

  template<typename T1, typename T2>
  parser<T1> first(parser<std::pair<T1, T2>> p) {
    return index<0>(p);
  }

  template<typename T1, typename T2>
  parser<T2> second(parser<std::pair<T1, T2>> p) {
    return index<1>(p);
  }

}

#endif // BLACK_PARSING_FUNCTIONAL_HPP
