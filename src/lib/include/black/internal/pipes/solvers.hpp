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

    virtual void set(std::string, std::string) override { }
    virtual void set(option, std::string) override { }

    virtual pipes::consumer *consumer() override { return this; }

    virtual support::tribool check() override { return V; }

    virtual std::optional<logic::model> model() const override {
      if constexpr (V)
        return {logic::model{}};
      else
        return {};
    }
  };

  inline constexpr auto sat = make_solver<const_t<true>>;
  inline constexpr auto unsat = make_solver<const_t<false>>;

  class preprocessed_t : 
    public solver::base, 
    public logic::model::base,
    public std::enable_shared_from_this<preprocessed_t>
  {
  private:
    virtual 
    std::optional<logic::term> value(logic::object x) const override {
      auto y = _pipe->translate(x);
      auto m = _slv.model();
      
      black_assert(m.has_value());
      
      auto v = m->value(y ? *y : x);

      if(!v)
        return {};

      if(y)
        return _pipe->undo(*v);
      
      return *v;
    }
    
    virtual 
    std::optional<logic::term> value(logic::object x, size_t t) const override {
      auto y = _pipe->translate(x);
      auto m = _slv.model();
      
      black_assert(m.has_value());
      
      auto v = m->value(y ? *y : x, t);

      if(!v)
        return {};

      if(y)
        return _pipe->undo(*v);
      
      return *v;
    }

    virtual size_t size() const override {
      black_assert(_slv.model().has_value());

      return _slv.model()->size();
    }

  public:
    preprocessed_t(pipes::transform::pipeline pipe, solver slv)
      : _slv{std::move(slv)}, _pipe{pipe(_slv.ptr()->consumer())} { }

    virtual ~preprocessed_t() override { }

    virtual void set(std::string option, std::string value) override { 
      _slv.set(option, value);
    }
    
    virtual void set(option option, std::string value) override { 
      _slv.set(option, value);
    }

    virtual pipes::consumer *consumer() override { return _pipe->consumer(); }
    
    virtual support::tribool check() override { 
      return _slv.ptr()->check(); 
    }
    
    virtual std::optional<logic::model> model() const override {
      if(_slv.model().has_value())
        return logic::model{this->shared_from_this()};

      return {};
    }

  private:
    solver _slv;
    pipes::transform::instance _pipe;
  };

  inline constexpr auto preprocessed = make_solver<preprocessed_t>;

}


namespace black::pipes {

  inline solvers::solver 
  operator|(pipes::transform::pipeline pipe, solvers::solver slv) {
    return solvers::preprocessed(std::move(pipe), std::move(slv));
  }
  

}

#endif // BLACK_PIPES_SOLVERS_HPP
