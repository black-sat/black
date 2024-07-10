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

#ifndef BLACK_SOLVERS_SOLVER_HPP
#define BLACK_SOLVERS_SOLVER_HPP

namespace black::solvers {

  //! Enum of known solver-agnostic options that can be passed to solvers.
  enum class option : uint8_t {
    logic //!< Set the SMT logic.
  };

  //!
  //! Opaque handle type representing solvers.
  //!
  //! Solvers can be instantiated in various ways (see e.g., `backends::cvc5`).
  //! Independently from their origin, this class wraps solver instances and
  //! provides a unified interface.
  //!
  class solver
  {
  public:
    class base;

    //! @name Constructor
    //! @{

    //! Constructs a \ref solver from a shared pointer to \ref solver::base.
    solver(std::shared_ptr<base> p);

    //! @}

    solver(solver const&) = default;
    solver(solver &&) = default;
    
    solver &operator=(solver const&) = default;
    solver &operator=(solver &&) = default;

    bool operator==(solver const&) const = default;

    //! Returns the solver's underlying pointer to \ref solver::base
    std::shared_ptr<base> ptr() const;

    //! Sets solver-specific options
    void set(std::string option, std::string value);

    //! Sets solver-agnostic options
    void set(option opt, std::string value);

    //! Check the satisfiability of the given module
    support::tribool check(logic::module mod);

    //! Ask the value of an object in the solver's current model.
    //! This works only after a call to \ref check() that returned true.
    std::optional<model> model() const;

  private:
    std::shared_ptr<base> _ptr;
    logic::module _last;
  };

  //!
  //! Virtual base class to be implemented by solvers.
  //!
  class solver::base {
  public:
    virtual ~base() = default;

    //! Sets solver-specific options
    virtual void set(std::string option, std::string value) = 0;

    //! Sets solver-agnostic options
    virtual void set(option opt, std::string value) = 0;

    //! Returns the solver's underlying \ref pipes::consumer
    virtual pipes::consumer *consumer() = 0;

    //! Check the satisfiability of the current solver's assertions stack
    virtual support::tribool check() = 0;

    //! Ask the value of an object in the solver's current model.
    //! This works only after a call to \ref check() that returned true.
    virtual std::optional<class model> model() const = 0;
  };

  inline solver::solver(std::shared_ptr<base> p) : _ptr{p} { }

  inline std::shared_ptr<solver::base> solver::ptr() const {
    return _ptr;
  }

  inline void solver::set(std::string opt, std::string value) {
    _ptr->set(opt, value);
  }

  inline void solver::set(option opt, std::string value) {
    _ptr->set(opt, value);
  }

  inline support::tribool solver::check(logic::module mod) {
    mod.replay(_last, _ptr->consumer());
    _last = mod;
    return _ptr->check();
  }

  inline std::optional<model> solver::model() const {
    return _ptr->model();
  }

  //!
  //! Helper type to declare a new solver factory object.
  //!
  template<typename S>
  inline constexpr auto make_solver = [](auto ...args) {
    return solver{std::make_shared<S>(std::move(args)...)};
  };
}

#endif // BLACK_SOLVERS_SOLVER_HPP
