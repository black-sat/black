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
    support::map<symbol, term> decls;
    support::map<symbol, term> defs;
  };

  module::module(alphabet *sigma) 
    : _impl{std::make_unique<_impl_t>(_impl_t{sigma, nullptr, {}, {}})} { } 
  
  module::module(scope const *base) 
    : _impl{std::make_unique<_impl_t>(_impl_t{base->sigma(), base, {}, {}})} 
    { } 
  
  module::~module() = default;

  alphabet *module::sigma() const {
    return _impl->sigma;
  }

  std::optional<scope::lookup> module::decl_of(symbol s) const {
    if(auto it = _impl->decls.find(s); it != _impl->decls.end())
      return lookup{s, it->second, this};
    
    if(_impl->base)
      return _impl->base->decl_of(s);
    
    return {};
  }
  
  std::optional<scope::lookup> module::def_of(symbol s) const {
    if(auto it = _impl->defs.find(s); it != _impl->defs.end())
      return lookup{s, it->second, this};

    if(_impl->base)
      return _impl->base->def_of(s);

    return {};
  }

  scope::result<void> module::declare(symbol s, term ty) {
    result<bool> ist = is_type(ty);
    if(!ist)
      return ist.error();
    if(!*ist)
      return error("type of declaration is not a type");
    if(_impl->decls.contains(s))
      return error("symbol already declared");
    if(_impl->defs.contains(s))
      return error("symbol already defined");
    
    _impl->decls.insert({s, ty});
    return {};
  }

  scope::result<void> module::define(symbol s, term value) {
    if(_impl->decls.contains(s))
      return error("symbol already declared");
    if(_impl->defs.contains(s))
      return error("symbol already defined");
    
    _impl->defs.insert({s, value});
    return {};
  }

  scope::result<void> module::declare(decl d) {
    return declare(d.name, d.type);
  }
  
  scope::result<void> module::define(def d) {
    return define(d.name, d.value);
  }

  scope::result<void> module::declare(std::vector<decl> const& decls) {
    for(decl d : decls)
      if(auto r = declare(d); !r)
        return r;
    return {};
  }
  
  scope::result<void> module::define(std::vector<def> const& defs) {
    for(def d : defs)
      if(auto r = define(d); !r)
        return r;
    return {};
  }

  scope::result<void> 
  module::declare(symbol s, std::vector<term> params, term range) {
    return declare(s, function_type(params, range));
  }
    
  scope::result<void> 
  module::define(symbol s, std::vector<decl> params, term body) {
    return define(s, lambda(params, body));
  }

}

