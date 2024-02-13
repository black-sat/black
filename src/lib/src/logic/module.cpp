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
    support::map<symbol, std::shared_ptr<decl>> decls;
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

  std::optional<const decl *> module::lookup(symbol s) const {
    if(auto it = _impl->decls.find(s); it != _impl->decls.end())
      return it->second.get();
    
    if(_impl->base)
      return _impl->base->lookup(s);
    
    return {};
  }

  scope::result<void> module::declare(symbol s, term ty) {
    if(_impl->decls.contains(s))
      return error("symbol already declared or defined");
    
    _impl->decls.insert({s, scope::make_decl(s, ty, {})});
    return {};
  }

  //
  // what do we have to do here
  // 1. compute the type of the term
  // 2. look for the symbol only in the current module (not parent scopes)
  //    a. if it is present
  //       - check it's not already defined
  //       - check the type is the same, and set the definition
  //    b. if it is not present, create and insert a new decl
  scope::result<void> module::define(symbol s, term value) 
  {
    // TODO: assume value is already resolved
    result<term> type = type_of(value);
    if(!type)
      return type.error();
    
    if(auto it = _impl->decls.find(s); it != _impl->decls.end()) {
      if(it->second->def())
        return error("symbol already defined");
      
      if(it->second->type() != type)
        return error("symbol already declared with a different type");

      it->second->def(value);
      return {};
    }
    
    _impl->decls.insert({s, scope::make_decl(s, *type, value)});
    return {};
  }

  scope::result<void> module::declare(binding b) {
    return declare(b.name, b.target);
  }
  
  scope::result<void> module::define(binding b) {
    return define(b.name, b.target);
  }

  scope::result<void> module::declare(std::vector<binding> const& binds) {
    for(binding b : binds)
      if(auto r = declare(b); !r)
        return r;
    return {};
  }
  
  scope::result<void> module::define(std::vector<binding> const& binds) {
    for(binding b : binds)
      if(auto r = define(b); !r)
        return r;
    return {};
  }

  scope::result<void> 
  module::declare(symbol s, std::vector<term> params, term range) {
    return declare(s, function_type(params, range));
  }
    
  scope::result<void> 
  module::define(symbol s, std::vector<binding> params, term body) {
    return define(s, lambda(params, body));
  }

}

