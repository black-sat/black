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

#ifndef BLACK_PARSING_PARSER_HPP
#define BLACK_PARSING_PARSER_HPP

#include <black/support>

#include <coroutine>
#include <ranges>
#include <algorithm>
#include <exception>
#include <generator>
#include <print>

namespace black::parsing::internal
{
  //
  // Misc types and utilities
  //
  template<typename T>
  class parser;
  
  template<typename T>
  class context;
  
  template<typename T, typename = void>
  class parsed;

  using subrange = std::ranges::subrange<const char *>;

  template<typename P, typename T>
  struct is_parser_of : std::false_type { };

  template<typename T>
  struct is_parser_of<parser<T>, T> : std::true_type { };

  template<typename P, typename T>
  inline constexpr bool is_parser_of_v = is_parser_of<P, T>::value;

  template<typename P, typename T>
  concept parser_of = is_parser_of_v<P, T>;

  template<typename T>
  using grammar = std::function<parsed<T>()>;

  template<typename T, typename ...Ts>
  bool is(std::variant<Ts...> const& v) {
    return std::holds_alternative<T>(v);
  }

  //
  // Reasons for parsing failures
  //
  enum class failure {
    eof = -1,
    reject = 1
  };

  //
  // Working modality of the `choice` primitive
  //
  enum class greed {
    lazy = 0,
    greedy = 1
  };


  //
  // Running parsing context
  //
  template<typename T>
  class context 
  {
  public:
    context() = delete;
    context(grammar<T> g) 
      : _grammar{std::make_shared<grammar<T>>(std::move(g))}, 
        _parsed{std::make_shared<parsed<T>>((*_grammar)())} { }

    context(context const&) = default;
    context(context &&) = default;
    
    context &operator=(context const&) = default;
    context &operator=(context &&) = default;

    std::expected<typename parser<T>::return_type, failure> 
    parse(subrange input, const char **tail = nullptr, greed g = greed::lazy);

    auto operator()(
      subrange input, const char **tail = nullptr, greed g = greed::lazy
    ) {
      return parse(input, tail, g);
    }

    std::expected<typename parser<T>::return_type, failure> 
    parse(
      std::string_view input, const char **tail = nullptr, 
      greed g = greed::lazy
    ) {
      return parse(
        subrange{
          std::to_address(std::begin(input)), 
          std::to_address(std::end(input))
        }, 
        tail, g
      );
    }

  private:
    std::shared_ptr<grammar<T>> _grammar;
    std::shared_ptr<parsed<T>> _parsed;
  };
    
  //
  // Coroutine type
  //
  template<typename T, typename>
  class [[nodiscard("parsed values must be awaited")]] parsed 
  {
  public:
    struct promise_type;

    parsed() = delete;

    parsed(parsed const&) = delete;
    parsed(parsed &&) = default;
    
    parsed &operator=(parsed const&) = delete;
    parsed &operator=(parsed &&) = default;

    std::coroutine_handle<promise_type> coroutine() { 
      return _coroutine.get();
    }

  private:
    parsed(std::coroutine_handle<promise_type> h) : _coroutine{ h } { }
    support::coroutine_handle_ptr<promise_type> _coroutine;
  };

  //
  // Main parser type
  //
  template<typename T>
  class parser
  {
  public:
    using return_type =
      std::conditional_t<
        std::is_unbounded_array_v<T>,
        std::vector<std::remove_extent_t<T>>,
        T
      >;

    template<std::invocable F>
      requires std::convertible_to<std::invoke_result_t<F>, parsed<T>>
    parser(F grammar) : _grammar{std::move(grammar)} { }
      
    parser(parser const&) = default;
    parser(parser &&) = default;
    
    parser &operator=(parser const&) = default;
    parser &operator=(parser &&) = default;

    context<T> start() { return { _grammar }; }

    template<typename ...Args>
      requires requires(context<T> c, Args ...args) { { c.parse(args...) }; }
    std::expected<return_type, failure> 
    parse(Args ...args) {
      return start().parse(args...);
    }

    template<typename ...Args>
      requires requires(context<T> c, Args ...args) { { c.parse(args...) }; }
    std::expected<return_type, failure> 
    operator()(Args ...args) {
      return start().parse(args...);
    }

    template<typename U>
      requires std::convertible_to<T, U>
    operator parser<U>() const {
      return [p = *this] -> parsed<U> {
        co_return static_cast<U>(co_await p);
      };
    }

    template<typename U>
      requires (!std::convertible_to<T, U> && std::is_constructible_v<U, T>)
    explicit operator parser<U>() const {
      return [p = *this] -> parsed<U> {
        co_return U(co_await p);
      };
    }

  private:
    grammar<T> _grammar;
  };

  namespace primitives {
    struct reject { };
    struct peek { };
    struct advance { };

    template<typename T>
    struct choice { 
      parser<T> first;
      parser<T> second;
    };

    template<typename T>
    struct try_ {
      parser<T> tried;
    };
  }

  namespace states {
    using resumer_t = 
      std::function<
        std::expected<std::any, failure>(subrange, const char **, greed)
      >;

    template<typename F>
    auto erase(F f) {
      return [=](auto ...args) mutable {
        return f(args...).transform(
          support::overload {
            []() { return std::any{ }; },
            [](auto v) { return std::any(std::move(v)); }
          }
        );
      };
    }

    struct reject { };
    
    struct eof { };

    struct ready { 
      std::any result;
    };

    struct awaiting {
      template<typename F>
      awaiting(F f) : resumer{ erase(f) } { }
      
      resumer_t resumer;
    };

    struct choice {
      template<typename F1, typename F2>
      choice(F1 f1, F2 f2) : first{ erase(f1) }, second{ erase(f2) } { }

      resumer_t first;
      resumer_t second;
    };

    struct completed { };

    struct unwinding { 
      std::exception_ptr exception;
    };
    
    using state = 
      std::variant<
        reject,
        eof,
        ready,
        awaiting,
        choice,
        completed,
        unwinding
      >;
  }

  struct machine 
  {
    states::state state = states::ready{};
    const char *input;
    const char *end;
    enum greed mode = greed::lazy;

    machine() = default;

    void step() {
      state = std::visit(
        [&](auto s) { return step(std::move(s)); }, 
        std::move(state)
      );
    }

    states::state step(states::unwinding u) { return u; }

    states::state step(states::reject) {
      return states::reject{}; 
    }

    states::state step(states::eof) { 
      if(input == end)
        return states::eof{};
      
      return states::ready{ };
    }

    states::state step(states::ready r) { 
      return r;
    }

    states::state step(states::awaiting a) {
      auto result = a.resumer({input, end}, &input, mode);
      if(result)
        return states::ready{ *result };
      
      if(result.error() == failure::reject)
        return states::reject{};
      
      return a;
    }

    states::state step(states::choice ch) {
      const char *saved = input;
      auto result = ch.first({input, end}, &input, mode);
      if(result)
        return states::ready{ *result };

      if(result.error() == failure::eof && mode == greed::greedy)
        return ch;

      if(saved == input)
        return step(states::awaiting{ ch.second });

      return states::reject{ };
    }

    states::state step(states::completed) { 
      return states::completed{};
    }
  };

  template<typename T>
  struct return_or_suspend 
  {
    using value_type = typename parser<T>::return_type;
    std::optional<value_type> value;

    return_or_suspend() = default;
    return_or_suspend(value_type v) : value{ v } { }

    bool await_ready() { return value.has_value(); }

    void await_suspend(std::coroutine_handle<>) { }
    
    value_type await_resume() { return std::move(*value); }
  };

  struct suspend_if {
    bool fail;

    suspend_if(bool f) : fail{ f } { }

    bool await_ready() { return !fail; }

    void await_suspend(std::coroutine_handle<>) { }
    
    void await_resume() {  }
  };

  struct peek_or_suspend {
    struct machine *machine;

    bool await_ready() { return machine->input != machine->end; }

    void await_suspend(std::coroutine_handle<>) { }
    
    char await_resume() { return *machine->input; }
  };

  struct advance_or_suspend {
    struct machine *machine;

    bool await_ready() { return (bool)machine->input; }

    void await_suspend(std::coroutine_handle<>) { }
    
    void await_resume() { /* machine->input.advance(1); */ }
  };

  template<typename T>
  struct awaiter {
    struct machine *machine;

    bool await_ready() { return is<states::ready>(machine->state); }

    void await_suspend(std::coroutine_handle<>) { }
    
    auto await_resume() { 
      if constexpr(!std::is_void_v<T>) {
        std::any value = 
          std::move(std::get<states::ready>(machine->state).result);

        using R = typename parser<T>::return_type;
        return std::any_cast<R>(value);
      }
    }
  };

  template<typename T, typename = void>
  struct promise_base {
    std::optional<T> _result;

    T result() { return std::move(*_result); }

    auto self() { return static_cast<parsed<T>::promise_type *>(this); }

    void return_value(T v) {
      _result = std::move(v);

      self()->machine.state = states::completed{ };
    }
  };

  template<typename Dummy>
  struct promise_base<void, Dummy> {
    auto self() { return static_cast<parsed<void>::promise_type *>(this); }

    void return_void() {
      self()->machine.state = states::completed{ };
    }
  };
  
  template<typename T>
  struct promise_base<T[]> {
    std::vector<T> _result;

    std::vector<T> result() { return std::move(_result); }

    auto self() { return static_cast<parsed<T[]>::promise_type *>(this); }

    void return_void() {
      self()->machine.state = states::completed{ };
    }

    std::suspend_never yield_value(T v) {
      _result.push_back(std::move(v));
      
      self()->machine.state = states::ready{ };

      return { };
    }

    template<std::ranges::range R>
      requires std::convertible_to<std::ranges::range_value_t<R>, T>
    std::suspend_never yield_value(std::ranges::elements_of<R> e) {
      std::ranges::move(
        std::begin(e.range),
        std::end(e.range),
        std::back_inserter(_result)
      );

      return { };
    }
  };

  template<typename T, typename Dummy>
  struct parsed<T, Dummy>::promise_type : promise_base<T, Dummy>
  {
    struct machine machine;

    // FIXME: Ask on SO why this is needed both on GCC and Clang
    promise_type() = default;

    auto get_return_object() {
      return 
        parsed<T>{ std::coroutine_handle<promise_type>::from_promise(*this) }; 
    }

    std::suspend_always initial_suspend() { return {}; }
    
    void unhandled_exception() {
      machine.state = states::unwinding{ std::current_exception() };
    }
    
    std::suspend_always final_suspend() noexcept { return {}; }

    template<typename U>
    awaiter<U> await_transform(parser<U> p) {
      machine.state = states::awaiting{ p.start() };
      machine.step();
      return { &machine };
    }

    std::suspend_always await_transform(primitives::reject) {
      machine.state = states::reject{};
      return {};
    }

    peek_or_suspend await_transform(primitives::peek) {
      machine.state = states::eof{};
      machine.step();
      return { &machine };
    }

    suspend_if await_transform(primitives::advance) {
      if(machine.input) {
        machine.input++;
        return false;
      }

      machine.state = states::eof{};
      return true;
    }

    template<typename U>
    awaiter<U> await_transform(primitives::choice<U> ch) 
    {
      machine.state = states::choice{ ch.first.start(), ch.second.start() };
      machine.step();
      return { &machine };
    }

    template<typename U>
    return_or_suspend<U> await_transform(primitives::try_<U> t) {
      const char *saved = machine.input;
      machine.state = states::awaiting{ t.tried.start() };
      machine.step();

      using R = typename parser<U>::return_type;
      if(is<states::ready>(machine.state))
        return std::any_cast<R>(std::get<states::ready>(machine.state).result);

      machine.input = saved;

      return { };
    }

  };

  template<typename T>
  std::expected<typename parser<T>::return_type, failure> 
  context<T>::parse(subrange input, const char **tail, greed g) {
    auto coroutine = _parsed->coroutine();
    auto *promise = &coroutine.promise();
    auto *machine = &promise->machine;
    
    machine->mode = g;
    machine->input = std::begin(input);
    machine->end = std::end(input);
    do{ 
      machine->step();
      
      if(is<states::ready>(machine->state))
        coroutine.resume();

      if(tail)
          *tail = machine->input;
      
      if(is<states::reject>(machine->state))
        return std::unexpected(failure::reject);
      
      if(is<states::eof>(machine->state))
        return std::unexpected(failure::eof);
      
      if(is<states::awaiting>(machine->state))
        return std::unexpected(failure::eof);
      
      if(is<states::choice>(machine->state))
        return std::unexpected(failure::eof);

      if(is<states::unwinding>(machine->state))
        std::rethrow_exception(
          std::get<states::unwinding>(machine->state).exception
        );

    } while(!is<states::completed>(machine->state));

    if constexpr(std::is_void_v<T>)
      return { };
    else 
      return { promise->result() };
  }

}

namespace black::parsing {
  using internal::subrange;
  using internal::failure;
  using internal::greed;
  using internal::parser;
  using internal::parsed;
  using internal::context;
  using internal::parser_of;
  using std::ranges::elements_of;
}

#endif // BLACK_PARSING_PARSER_HPP
