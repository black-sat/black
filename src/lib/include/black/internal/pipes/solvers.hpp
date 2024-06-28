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

#ifndef BLACK_PIPES_SOLVERS_HPP
#define BLACK_PIPES_SOLVERS_HPP

namespace black::solvers {

  using pipes::solver;
  
  template<bool V>
  class const_t : public solver::base, private pipes::consumer
  {
  public:
    const_t() = default;

    virtual void import(logic::module) override { }
    virtual void adopt(std::shared_ptr<logic::root const>) override { }
    virtual void state(logic::term, logic::statement) override { }
    virtual void push() override { }
    virtual void pop(size_t) override { }

    void set_smt_logic(std::string const&) override { }

    virtual pipes::consumer *consumer() override { return this; }

    virtual support::tribool check() override { return V; }

    virtual std::optional<logic::term> value(logic::object) override {
      return {};
    }
  };

  inline constexpr auto sat = pipes::make_solver<const_t<true>>{};
  inline constexpr auto unsat = pipes::make_solver<const_t<false>>{};


  class preprocessed_t : public solver::base
  {
  public:
    preprocessed_t(pipes::transform::pipeline pipe, solver::pipeline slv)
      : _slv{slv()}, _pipe{pipe(_slv->consumer())} { }

    void set_smt_logic(std::string const& logic) override {
      _slv->set_smt_logic(logic);
    }

    virtual pipes::consumer *consumer() override { return _pipe->consumer(); }
    
    virtual support::tribool check() override { return _slv->check(); }
    
    virtual std::optional<logic::term> value(logic::object x) override {
      auto y = _pipe->translate(x);
      auto v = _slv->value(y ? *y : x);

      if(!v)
        return {};

      if(y)
        return _pipe->undo(*v);
      
      return *v;
    }

  private:
    solver::instance _slv;
    pipes::transform::instance _pipe;
  };

  inline constexpr auto preprocessed = pipes::make_solver<preprocessed_t>{};

}


namespace black::pipes {

  inline solver::pipeline
  operator|(pipes::transform::pipeline pipe, solver::pipeline slv) {
    return solvers::preprocessed(std::move(pipe), std::move(slv));
  }
  

}

#endif // BLACK_PIPES_SOLVERS_HPP
