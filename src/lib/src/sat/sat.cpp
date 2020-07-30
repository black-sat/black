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

#include <black/sat/sat.hpp>

#include <tsl/hopscotch_map.h>
#include <string>

namespace black::sat
{
  namespace internal {
    namespace {
      tsl::hopscotch_map<const char *, backend_init_hook::backend_ctor> 
        backends;
    }
    
    backend_init_hook::backend_init_hook(const char *name, backend_ctor ctor)
    {
      black_assert(backends.find(name) == backends.end());
      backends.insert({name, ctor});
    }
  }

  bool solver::has_solver(const char *name) {
    using namespace black::sat::internal;
    
    return backends.find(name) != backends.end();
  }

  bool solver::has_solver(std::string const &name) {
    return has_solver(name.c_str());
  }

  std::unique_ptr<solver> solver::get_solver(const char *name) 
  {
    using namespace black::sat::internal;
    auto it = backends.find(name); 
    
    black_assert(it != backends.end());
    
    return (it->second)();
  }

  std::unique_ptr<solver> solver::get_solver(std::string const& name) {
    return get_solver(name.c_str());
  }
}
