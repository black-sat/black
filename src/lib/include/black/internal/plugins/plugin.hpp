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

#ifndef BLACK_INTERNAL_PLUGIN_HPP
#define BLACK_INTERNAL_PLUGIN_HPP

#include <memory>
#include <string_view>

namespace black::plugins {

  class registry;

  class plugin 
  {
  public:
    plugin(class registry *reg) : _registry{ reg } { }
    virtual ~plugin() = default;
    
    plugin(plugin const&) = delete;
    plugin(plugin &&) = delete;
    
    plugin &operator=(plugin const&) = delete;
    plugin &operator=(plugin &&) = delete;

    class registry *registry() const { return _registry; }

    virtual 
    std::unique_ptr<plugin const> 
    load(class registry *, std::string const&) const = 0;

  private:
    class registry *_registry;
  };

  class registry 
  {
    struct _private { };
  public:
    static registry *instance();

    registry(_private);
    ~registry();

    registry(registry const&) = delete;
    registry(registry &&) = delete;

    registry &operator=(registry const&) = delete;
    registry &operator=(registry &&) = delete;

    bool load(std::string const& name);

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

}

#endif // BLACK_INTERNAL_PLUGIN_HPP
