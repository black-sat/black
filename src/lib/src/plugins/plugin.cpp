//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2025 Nicola Gigante
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

#include <black/plugins>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace black::plugins {

  struct root : plugin 
  {
    root(class registry *reg) : plugin{reg} {}
    virtual ~root() override = default;
    
    virtual 
    std::unique_ptr<plugin const> 
    load(class registry *, std::string const&) const override {
      return nullptr;
    }

  };

  struct registry::_impl_t {
    std::unordered_map<std::string, std::unique_ptr<plugin const>> plugins;
  };

  static std::unique_ptr<registry> _instance = nullptr;
  static std::mutex _instance_lock;

  registry *registry::instance() {
    std::scoped_lock lock(_instance_lock);

    if(!_instance)
      _instance = std::make_unique<registry>(_private{});

    return _instance.get();
  }

  registry::~registry() = default;

  registry::registry(_private) : _impl{std::make_unique<_impl_t>()} { 
    _impl->plugins.insert({ "root", std::make_unique<root>(this) });
  }

  bool registry::load(std::string const& name) {
    for(auto &p : _impl->plugins) {
      if(auto loaded = p.second->load(this, name); loaded) {
        _impl->plugins.insert({ name, std::move(loaded) });
        return true;
      }
    }

    return false;
  }


}