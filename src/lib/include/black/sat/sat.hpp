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

#include <black/logic/formula.hpp>

#include <memory>
#include <type_traits>
#include <string_view>
#include <vector>

namespace black::sat 
{  

  //
  // Generic interface to backend SAT solvers
  //  
  class solver 
  {
  public:
    // default constructor
    solver() = default;

    static std::vector<std::string_view> backends();
    static bool backend_exists(std::string_view name);
    static std::unique_ptr<solver> get_solver(std::string_view name);

    // solver is a polymorphic, non-copyable type
    solver(const solver &) = delete;
    solver &operator=(const solver &) = delete;

    virtual ~solver() = default;

    // assert a formula, adding it to the current context
    virtual void assert_formula(formula f) = 0;

    // tell if the current set of assertions is satisfiable, 
    // under the given assumptions
    virtual bool is_sat(std::vector<formula> const &assumptions) = 0;

    // tell if the current set of assertions is satisfiable
    virtual bool is_sat() { return is_sat(std::vector<formula>{}); }

    // tell if the current set of assertions is satisfiable, 
    // under the given assumption
    virtual bool is_sat(formula assumption) { 
      return is_sat(std::vector<formula>{assumption}); 
    }
    
    // clear the current context completely
    virtual void clear() = 0;
  };

  namespace internal {
    struct backend_init_hook {
      using backend_ctor = std::unique_ptr<solver> (*)();
      backend_init_hook(std::string_view, backend_ctor);
    };

    #define BLACK_REGISTER_SAT_BACKEND(Backend) \
      static const black::sat::internal::backend_init_hook \
        Backend##_init_hook_{ \
          #Backend, \
          []() -> std::unique_ptr<::black::sat::solver> { \
            return std::make_unique<::black::sat::backends::Backend>(); \
          } \
        };
  }
}


#endif
