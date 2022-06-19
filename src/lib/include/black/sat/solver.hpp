//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#ifndef BLACK_SAT_SOLVER_HPP
#define BLACK_SAT_SOLVER_HPP

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>
#include <black/support/tribool.hpp>

#include <memory>
#include <type_traits>
#include <string_view>
#include <vector>

namespace black::sat 
{
  enum class feature {
    smt,
    quantifiers
  };

  //
  // Generic interface to backend SAT solvers
  //  
  class BLACK_EXPORT solver 
  {
  public:

    // default constructor
    solver() = default;

    static std::vector<std::string_view> backends();
    static bool backend_exists(std::string_view name);
    static std::unique_ptr<solver> get_solver(std::string_view name);
    static bool backend_has_feature(std::string_view name, feature f);

    // solver is a polymorphic, non-copyable type
    solver(const solver &) = delete;
    solver &operator=(const solver &) = delete;

    virtual ~solver() = default;

    // assert a formula, adding it to the current context
    virtual void assert_formula(logic::formula<FO> f) = 0;

    // tell if the current set of assertions is satisfiable
    virtual tribool is_sat() = 0;
    
    // tell if the current set of assertions is satisfiable, 
    // under the given assumption
    virtual tribool is_sat_with(logic::formula<FO> assumption) = 0;
    
    // gets the value of a proposition from the solver.
    // The result is tribool::undef if the variable has not been decided
    // e.g. before the first call to is_sat()
    virtual tribool value(proposition a) const = 0;

    // clear the current context completely
    virtual void clear() = 0;

    // License note for whatever third-party software lies under the hood
    virtual std::optional<std::string> license() const = 0;
  };

  namespace internal {
    struct backend_init_hook {
      using backend_ctor = std::unique_ptr<solver> (*)();
      backend_init_hook(
        std::string_view, backend_ctor, std::vector<black::sat::feature>
      );
    };

    #define BLACK_REGISTER_SAT_BACKEND(Backend, ...) \
      static const black::sat::internal::backend_init_hook \
        Backend##_init_hook_{ \
          #Backend, \
          []() -> std::unique_ptr<::black::sat::solver> { \
            return std::make_unique<::black::sat::backends::Backend>(); \
          }, \
          __VA_ARGS__ \
        };
  }
}


#endif
