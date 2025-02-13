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

namespace black::parsing::internal
{
  //
  // Misc types and traits
  //
  template<typename T>
  class parser;
  
  template<typename T>
  class context;
  
  template<typename T, typename = void>
  class parsed;

  using range = std::ranges::subrange<const char *>;

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

  //
  // Reasons for parsing failures
  //
  enum class failure {
    eof = -1,
    reject = 1
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
      : _grammar{std::make_unique<grammar<T>>(std::move(g))}, 
        _parsed{(*_grammar)()} { }

    context(context const&) = delete;
    context(context &&) = default;
    
    context &operator=(context const&) = delete;
    context &operator=(context &&) = default;

    std::expected<T, failure> parse(range input, range *tail = nullptr);

  private:
    std::unique_ptr<grammar<T>> _grammar;
    parsed<T> _parsed;
  };
  
  //
  // Specialization of parsing context for sequential parsers
  //
  template<typename T>
  class context<T[]>
  {
  public:
    context() = delete;
    context(grammar<T[]> g) 
      : _grammar{std::make_unique<grammar<T[]>>(std::move(g))}, 
        _parsed{(*_grammar)()} { }

    context(context const&) = delete;
    context(context &&) = default;
    
    context &operator=(context const&) = delete;
    context &operator=(context &&) = default;

    std::expected<std::vector<T>, failure>
    parse(range input, range *tail = nullptr);

  private:
    std::unique_ptr<grammar<T[]>> _grammar;
    parsed<T[]> _parsed;
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
    using output_type =
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

    std::expected<output_type, failure> 
    parse(range input, range *tail = nullptr) {
      return start().parse(input, tail);
    }

    template<typename U>
      requires (!std::is_void_v<T>)
    operator parser<U>() const {
      return [p = *this] -> parsed<U> {
        co_return static_cast<U>(co_await p);
      };
    }

    operator parser<std::any>() const requires std::is_void_v<T> {
      return [p = *this] -> parsed<std::any> {
        co_await p;
        co_return std::any{};
      };
    }

  private:
    grammar<T> _grammar;
  };

  namespace primitives {
    struct reject { };
    struct eof { };
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
    struct reject { };
    struct eof { };
    struct peek { };
    struct advance { };

    struct choice {
      context<std::any> first;
      parser<std::any> second;
    };

    struct try_ {
      context<std::any> tried;
    };
    
    struct ready { 
      std::any result;
    };
    
    struct awaiting { 
      context<std::any> awaited;
    };

    struct completed { };

    using state = 
      std::variant<
        reject,
        eof,
        peek,
        advance,
        choice,
        try_,
        ready,
        awaiting,
        completed
      >;
  }

  struct machine 
  {
    states::state state = states::ready{};
    range input;

    machine() = default;

    void step() {
      state = std::visit(
        [&](auto s) { return step(std::move(s)); }, 
        std::move(state)
      );
    }

    states::state step(states::reject) { return states::reject{}; }

    states::state step(states::eof) { 
      if(!input)
        return states::eof{};
      
      return states::ready{};
    }

    states::state step(states::peek) {
      if(!input)
        return states::peek{};
      
      return states::ready{ *std::begin(input) };
    }

    states::state step(states::advance) {
      if(!input)
        return states::advance{};
      
      input.advance(1);
      return states::ready{};
    }

    states::state step(states::choice ch) {
      range saved = input;
      auto result = ch.first.parse(input, &input);
      if(result)
        return states::ready{ *result };
      
      if(std::begin(saved) != std::begin(input))
          return states::reject{};

      return states::awaiting{ ch.second.start() };
    }

    states::state step(states::try_ t) {
      range saved = input;
      auto result = t.tried.parse(input, &input);
      if(result)
        return states::ready{ *result };

      input = saved;
      return states::reject{ };
    }

    states::state step(states::ready r) { 
      return r;
    }

    states::state step(states::awaiting a) {
      auto result = a.awaited.parse(input, &input);
      if(result)
        return states::ready{ *result };
      
      if(result.error() == failure::reject)
          return states::reject{};
      
      return std::move(a);
    }

    states::state step(states::completed) { return states::completed{}; }
  };

  template<typename T, typename U>
  struct awaiter 
  {
    parsed<T>::promise_type *promise;

    bool await_ready() { 
      promise->machine.step();
      return std::holds_alternative<states::ready>(promise->machine.state);
    }

    void await_suspend(std::coroutine_handle<>) { }
    
    typename parser<U>::output_type await_resume() {
      if constexpr (!std::is_void_v<U>)
        return std::move(
          std::any_cast<typename parser<U>::output_type>(
            std::get<states::ready>(promise->machine.state).result
          )
        );
    }
  };

  template<typename T, typename = void>
  struct promise_base {
    std::optional<T> returned;

    void return_value(T v) {
      returned = std::move(v);

      auto promise = static_cast<parsed<T>::promise_type *>(this);
      promise->machine.state = states::completed{ };
    }
  };

  template<typename Dummy>
  struct promise_base<void, Dummy> {
    void return_void() {
      auto promise = static_cast<parsed<void, Dummy>::promise_type *>(this);
      promise->machine.state = states::completed{ };
    }
  };
  
  template<typename T>
  struct promise_base<T[]> {
    std::vector<T> yields;

    void return_void() {
      auto promise = static_cast<parsed<T[]>::promise_type *>(this);
      promise->machine.state = states::completed{ };
    }

    std::suspend_always yield_value(T v) {
      yields.push_back(std::move(v));
      
      auto promise = static_cast<parsed<T[]>::promise_type *>(this);
      promise->machine.state = states::ready{ };

      return {};
    }
  };

  template<typename T, typename Dummy>
  struct parsed<T, Dummy>::promise_type : promise_base<T, Dummy>
  {
    machine machine;

    // FIXME: Ask on SO why this is needed both on GCC and Clang
    promise_type() = default;

    auto get_return_object() {
      return 
        parsed<T>{ std::coroutine_handle<promise_type>::from_promise(*this) }; 
    }

    auto initial_suspend() { return std::suspend_always{}; }
    
    void unhandled_exception() {}
    
    auto final_suspend() noexcept { return std::suspend_always{}; }

    template<typename U>
    auto await_transform(parser<U> p) {
      machine.state = states::awaiting{ parser<std::any>(p).start() };
      return awaiter<T, U>{ this };
    }

    auto await_transform(primitives::reject) {
      machine.state = states::reject{ };
      return std::suspend_always();
    }

    auto await_transform(primitives::eof) {
      machine.state = states::eof{ };
      return awaiter<T, void>{ this };
    }

    auto await_transform(primitives::peek) {
      machine.state = states::peek{ };
      return awaiter<T, char>{ this };
    }

    auto await_transform(primitives::advance) {
      machine.state = states::advance{ };
      return awaiter<T, void>{ this };
    }

    template<typename U>
    auto await_transform(primitives::choice<U> ch) {
      machine.state = states::choice{ 
        parser<std::any>(ch.first).start(), 
        parser<std::any>(ch.second)
      };
      return awaiter<T, U>{ this };
    }

    template<typename U>
    auto await_transform(primitives::try_<U> ch) {
      machine.state = states::try_{ parser<std::any>(ch.tried).start() };
      return awaiter<T, U>{ this };
    }

  };

  template<typename T>
  std::expected<T, failure> 
  context<T>::parse(range input, range *tail) {
    auto *promise = &_parsed.coroutine().promise();
    auto *machine = &promise->machine;
    machine->input = input;
    machine->step();
    
    if(std::holds_alternative<states::ready>(machine->state))
      _parsed.coroutine().resume();

    if(tail)
        *tail = machine->input;

    if(std::holds_alternative<states::reject>(machine->state))
      return std::unexpected(failure::reject);

    if(std::holds_alternative<states::completed>(machine->state)) {
      if constexpr(std::is_void_v<T>)
        return { };
      else
        return { *promise->returned };
    }

    return std::unexpected(failure::eof);
  }

  template<typename T>
  std::expected<std::vector<T>, failure> 
  context<T[]>::parse(range input, range *tail) {
    auto *promise = &_parsed.coroutine().promise();
    auto *machine = &promise->machine;
    
    machine->input = input;

    do {
      machine->step();

      if(std::holds_alternative<states::ready>(machine->state))
        _parsed.coroutine().resume();

      if(tail)
        *tail = machine->input;
      
      if(std::holds_alternative<states::reject>(machine->state))
        return std::unexpected(failure::reject);
      
      if(!std::holds_alternative<states::completed>(machine->state))
        continue;

      return std::unexpected(failure::eof);
    
    } while(true);

    return promise->yields;
  }

}

namespace black::parsing {
  using internal::range;
  using internal::parser;
  using internal::parsed;
  using internal::context;
  using internal::parser_of;
}

#endif // BLACK_PARSING_PARSER_HPP
