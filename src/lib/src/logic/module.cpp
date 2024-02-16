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

#include <black/support>
#include <black/logic>

namespace black::logic {

  struct module::_impl_t 
  {
    alphabet *sigma;
    scope const* base;
    support::map<label, std::shared_ptr<decl>> decls;
  };

  module::module(alphabet *sigma) 
    : _impl{std::make_unique<_impl_t>(_impl_t{sigma, nullptr, {}})} { } 
  
  module::module(scope const *base) 
    : _impl{std::make_unique<_impl_t>(_impl_t{base->sigma(), base, {}})} 
    { } 
  
  module::~module() = default;

  alphabet *module::sigma() const {
    return _impl->sigma;
  }

  std::shared_ptr<decl const> module::lookup(label s) const {
    if(auto it = _impl->decls.find(s); it != _impl->decls.end())
      return it->second;
    
    if(_impl->base)
      return _impl->base->lookup(s);
    
    return nullptr;
  }

  object module::declare(label s, term ty) {
    auto d = std::make_shared<decl>(decl{s, ty, {}});
    _impl->decls.insert({s, d});
    
    return _impl->sigma->object(d);
  }

  object module::declare(binding b) {
    return declare(b.name.name(), b.target);
  }

  object module::declare(label s, std::vector<term> params, term range) {
    return declare(s, function_type(params, range));
  }

  std::vector<object> module::declare(std::vector<binding> const& binds) {
    std::vector<object> vars;
    for(binding b : binds)
      vars.push_back(declare(b)); 

    return vars;
  }

  object module::define(label s, term type, term def)
  {
    auto d = std::make_shared<decl>(decl{s, type, def});
    _impl->decls.insert({s, d});

    return _impl->sigma->object(d);
  }

  object module::define(def d) {
    return define(d.name, d.type, d.def);
  }
  
  object 
  module::define(label s, std::vector<binding> params, term range, term body) {
    std::vector<term> paramtypes;
    for(auto p : params)
      paramtypes.push_back(p.target);
    
    auto type = function_type(paramtypes, range);
    return define(s, type, lambda(params, body));
  }
  
  std::vector<object> module::define(std::vector<def> const& defs) 
  {
    std::vector<object> vars;
    for(auto def : defs)
      vars.push_back(define(def));
  
    return vars;
  }
    
  void module::resolve() {
    support::map<label, term> defs;
    for(auto [name, decl] : _impl->decls)
      if(decl->def) 
        defs.insert({name, resolve(*decl->def)});

    for(auto [name, def] : defs)
      _impl->decls[name]->def = def;
  }


}

