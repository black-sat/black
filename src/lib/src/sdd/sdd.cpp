//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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
//

#include <black/sdd/sdd.hpp>
#include <black/sdd/sdd.h>

#include <tsl/hopscotch_map.h>

namespace black::sdd {

  struct manager::impl_t {
    impl_t(alphabet *_sigma) 
      : sigma{_sigma}, mgr{sdd_manager_create(1, false)} { }

    tsl::hopscotch_map<proposition, sdd::variable> vars;
    alphabet *sigma;
    SddManager *mgr;
  };

  manager::manager(alphabet *sigma) : _impl{std::make_unique<impl_t>(sigma)} { }

  manager::manager(manager &&) = default;
  manager &manager::operator=(manager &&) = default;
  manager::~manager() = default;

  variable manager::variable(proposition name) {
    if(_impl->vars.contains(name))
      return _impl->vars.at(name);

    SddLiteral var = sdd_manager_var_count(_impl->mgr);
    sdd_manager_add_var_after_last(_impl->mgr);

    return sdd::variable{this, name, unsigned(var)};
  }

}