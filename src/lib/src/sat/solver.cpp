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

#include <black/support/common.hpp>
#include <black/sat/solver.hpp>

#include <tsl/hopscotch_map.h>

namespace black::sat
{
  namespace internal {
    namespace {
      using backends_map = 
        tsl::hopscotch_map<
          std::string_view, 
          std::pair<
            backend_init_hook::backend_ctor, 
            std::vector<black::sat::feature>
          >
        >;
      
      std::unique_ptr<backends_map> _backends = nullptr;
    }
    
    backend_init_hook::backend_init_hook(
      std::string_view name, backend_ctor ctor, 
      std::vector<black::sat::feature> features
    ) {
      if(!_backends)
        _backends = std::make_unique<backends_map>();

      black_assert(_backends->find(name) == _backends->end());
      _backends->insert({name, {ctor, features}});
    }
  }

  bool solver::backend_exists(std::string_view name) {
    using namespace black::sat::internal;
    
    return _backends->find(name) != _backends->end();
  }

  std::unique_ptr<solver> solver::get_solver(
    std::string_view name, scope const& xi
  ) {
    using namespace black::sat::internal;
    auto it = _backends->find(name); 
    
    black_assert(it != _backends->end());
    
    return (it->second.first)(xi);
  }

  bool solver::backend_has_feature(std::string_view name, feature f)
  {
    using namespace black::sat::internal;
    auto it = _backends->find(name);

    black_assert(it != _backends->end());

    std::vector<feature> features = it->second.second;

    return std::find(features.begin(), features.end(), f) != features.end();
  }

  std::vector<std::string_view> solver::backends() {
    using namespace black::sat::internal;

    std::vector<std::string_view> result;

    for(auto [key,elem] : *_backends) {
      result.push_back(key);
    }

    return result;
  }
  
}
