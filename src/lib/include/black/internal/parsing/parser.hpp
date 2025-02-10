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

namespace black::parsing 
{
  //
  // Misc types and traits
  //
  template<typename T>
  struct parser;
  
  template<typename T>
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

  template<typename G, typename T>
  concept grammar_of = requires(G g) {
    { g() } -> std::convertible_to<parsed<T>>;
  };

  //
  // Core of the parser type
  //
  template<typename T>
  class runner
  {
  public:
    runner(grammar_of<T> auto grammar) : _grammar{std::move(grammar)} { }
      
    runner(runner const&) = default;
    runner(runner &&) = default;
    
    runner &operator=(runner const&) = default;
    runner &operator=(runner &&) = default;

    auto run(range input, range *tail = nullptr)
    {
      parsed<T> parsed = _grammar();

      parsed.coroutine()->promise().input = input;
      parsed.coroutine()->resume();
      if(tail)
        *tail = parsed.coroutine()->promise().input;

      return parsed.coroutine()->promise().value;
    }

  private:
    std::function<parsed<T>()> _grammar;
  };

    
  template<typename T>
  class runner<T[]>
  {
  public:
    runner(grammar_of<T[]> auto grammar) : _grammar{std::move(grammar)} { }
      
    runner(runner const&) = default;
    runner(runner &&) = default;
    
    runner &operator=(runner const&) = default;
    runner &operator=(runner &&) = default;

    std::optional<std::vector<std::remove_extent_t<T>>> 
    run(range input, range *tail = nullptr)
    {
      parsed<T[]> parsed = _grammar();

      parsed.coroutine()->promise().input = input;

      std::vector<std::remove_extent_t<T>> results;

      do {
        parsed.coroutine()->resume();
        auto value = std::move(parsed.coroutine()->promise().value);
        
        if(value)
          results.push_back(std::move(*value));
        
        if(!value && !parsed.coroutine()->done())
          return std::nullopt;

      } while(!parsed.coroutine()->done());

      if(tail)
        *tail = parsed.coroutine()->promise().input;

      return results;
    }
  
  private:
    std::function<parsed<T[]>()> _grammar;
  };

  //
  // Main parser type
  // 
  template<typename T>
  struct parser : runner<T> {
    using runner<T>::runner;
  };

  //
  // Types representing the primitive actions available to parsers
  //

  //
  // Type to return to cause a parsing failure
  //
  struct fail_t { };

  //
  // Succeed if we are at EOF, fail otherwise
  //
  struct eof_t { };

  //
  // peek the current token
  //
  struct peek_t { };

  //
  // advance the current position in the input
  //
  struct advance_t { };

  //
  // lift to an std::optional a parser that fails without consuming input
  //
  template<typename T>
  struct optional_t { 
    parser<T> inner;
  };

  //
  // Parse ahead without consuming input on failure
  //
  template<typename T>
  struct try_t {
    parser<T> inner;
  };

  //
  // Return type of parsing coroutines
  //
  template<typename T>
  class [[nodiscard("parsed values must be awaited")]] parsed 
  {
  public:
    struct promise_type;

    parsed() = delete;

    parsed(parsed const&) = delete;
    parsed(parsed &&) = default;
    
    parsed &operator=(parsed const&) = delete;
    parsed &operator=(parsed &&) = default;

    auto &coroutine() { return _coroutine; }

  private:
    parsed(std::coroutine_handle<promise_type> h) : _coroutine{ h } { }
    support::coroutine_handle_ptr<promise_type> _coroutine;
  };

  template<typename T>
  struct suspend_or_return {
    std::optional<T> _value = std::nullopt;

    suspend_or_return() = default;
    suspend_or_return(T v) : _value{v} { }
    suspend_or_return(std::optional<T> opt) : _value{opt} { }

    bool await_ready() { return _value.has_value(); }

    void await_suspend(std::coroutine_handle<>) { }
    
    T await_resume() { return std::move(*_value); }
  };
  
  struct suspend_if {
    bool _fail;

    suspend_if(bool succeed) : _fail{succeed} { }

    bool await_ready() { return !_fail; }

    void await_suspend(std::coroutine_handle<>) { }
    
    void await_resume() { }
  };

  template<typename T>
  struct promise_value_holder {
    std::optional<T> value = std::nullopt;

    promise_value_holder() = default;

    void return_value(T v) {
      value = std::move(v);
    }
  };

  template<typename T>
  struct promise_value_holder<T[]> {
    std::optional<T> value = std::nullopt;

    promise_value_holder() = default;

    void return_void() { value = std::nullopt; }

    std::suspend_always yield_value(T v) {
      this->value = std::move(v);
      return {};
    }
  };

  template<>
  struct promise_value_holder<void> {
    std::optional<std::monostate> value = std::nullopt;

    promise_value_holder() = default;

    void return_void() {
      value = std::monostate{};
    }
  };

  template<typename T>
  struct parsed<T>::promise_type : promise_value_holder<T>
  {
    range input;

    // FIXME: Ask on SO why this is needed both on GCC and Clang
    promise_type() = default;

    auto get_return_object() {
      return parsed<T>{ 
        std::coroutine_handle<promise_type>::from_promise(*this) 
      }; 
    }

    auto initial_suspend() { return std::suspend_always{}; }
    
    void unhandled_exception() {}
    
    auto final_suspend() noexcept { return std::suspend_always{}; }

    template<typename U>
    auto await_transform(parser<U> p) {
      return suspend_or_return<U>{ p.run(input, &input) };
    }

    template<typename U>
    auto await_transform(parser<U[]> p) {
      return suspend_or_return<std::vector<U>>{ p.run(input, &input) };
    }

    auto await_transform(parser<void> p) {
      return suspend_if{ ! p.run(input, &input).has_value() };
    }

    auto await_transform(fail_t) {
      return std::suspend_always{};
    }

    auto await_transform(eof_t) {
      return suspend_if{ ! input.empty() };
    }

    auto await_transform(peek_t) {
      if(!input)
        return suspend_or_return<char>{ };

      return suspend_or_return{ *std::begin(input) };
    }

    auto await_transform(advance_t) {
      if(!input)
        return suspend_if{ true };

      input.advance(1);
      return suspend_if{ false };
    }

    template<typename U>
    auto await_transform(optional_t<U> opt) {
      auto saved = input;
      auto result = opt.inner.run(input, &input);
      
      if(!result && std::begin(saved) != std::begin(input))
        return suspend_or_return<std::optional<U>>{ };
      
      return suspend_or_return<std::optional<U>>{ std::move(result) };
    }

    template<typename U>
    auto await_transform(try_t<U> opt) {
      auto saved = input;
      auto result = opt.inner.run(input, &input);
      if(result)
        return suspend_or_return<U>{ *result };
      
      input = saved;
      return suspend_or_return<U>{ };
    }

  };

  //
  // Parser type
  //
  template<typename Pred>
  concept predicate = requires (Pred p, char c) {
    { p(c) } -> std::convertible_to<bool>;
  };

}

#endif // BLACK_PARSING_PARSER_HPP
