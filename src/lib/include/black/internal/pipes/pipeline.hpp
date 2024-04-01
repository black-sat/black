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

#ifndef BLACK_PIPES_PIPELINE_HPP
#define BLACK_PIPES_PIPELINE_HPP

#include <black/support>
#include <black/logic>

#include <vector>
#include <array>

namespace black::pipes {

  template<typename P, typename T, typename ...Args>
  struct make_pipe {
    constexpr make_pipe() = default;
    
    make_pipe(make_pipe const&) = default;
    make_pipe(make_pipe &&) = default;

    make_pipe &operator=(make_pipe const&) = default;
    make_pipe &operator=(make_pipe &&) = default;

    template<typename ...Args2>
      requires std::is_constructible_v<P, Args..., Args2...>
    typename T::pipeline operator()(Args2 ...args2) const {
      return [... args3 = std::move(args2)](Args ...args) 
        -> typename T::instance 
      { 
        return std::make_unique<P>( std::move(args)..., std::move(args3)... );
      };
    }
  };

  class transform
  {
  public:
    class base;
    class common;
    using instance = std::unique_ptr<base>;
    using pipeline = std::function<instance(consumer *next)>;

    transform(pipeline p);

    transform(transform const&) = delete;
    transform(transform &&) = default;
    
    transform &operator=(transform const&) = delete;
    transform &operator=(transform &&) = default;

    base *get() { return _instance.get(); }

    logic::module operator()(logic::module mod);

  private:
    logic::module _last;
    logic::module _output;
    instance _instance;
  };

  class transform::base {
  public:
    virtual ~base() = default;

    virtual class consumer *consumer() = 0;

    virtual logic::object translate(logic::object) = 0;
  };

  inline transform::transform(pipeline p) : _instance{p(&_output)} { }

  inline logic::module transform::operator()(logic::module mod) {
    mod.replay(_last, _instance->consumer());
    _last = std::move(mod);
    return _output;
  }

  template<typename P>
  using make_transform = make_pipe<P, transform, consumer *>;

  //
  //
  //
  class solver
  {
  public:
    class base;
    using instance = std::unique_ptr<base>;
    using pipeline = std::function<instance()>;

    solver(pipeline p);

    solver(solver const&) = delete;
    solver(solver &&) = default;
    
    solver &operator=(solver const&) = delete;
    solver &operator=(solver &&) = default;

    base *get() { return _instance.get(); }

    support::tribool check(logic::module mod);
    std::optional<logic::term> value(logic::term t);

  private:
    logic::module _last;
    instance _instance;
  };

  class solver::base {
  public:
    virtual ~base() = default;

    virtual pipes::consumer *consumer() = 0;

    virtual support::tribool check() = 0;

    virtual std::optional<logic::term> value(logic::term) = 0;
  };

  inline solver::solver(pipeline p) : _instance{p()} { }

  inline support::tribool solver::check(logic::module mod) {
    mod.replay(_last, _instance->consumer());
    _last = std::move(mod);
    return _instance->check();
  }

  inline std::optional<logic::term> solver::value(logic::term t) {
    return _instance->value(t);
  }

  template<typename P>
  using make_solver = make_pipe<P, solver>;

}

#endif // BLACK_PIPES_PIPELINE_HPP
