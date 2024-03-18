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

#ifndef BLACK_LOGIC_SCOPE_HPP
#define BLACK_LOGIC_SCOPE_HPP

#include <optional>
#include <expected>
#include <memory>

#include <black/support>

namespace black::logic {

  using ast::core::label;

  struct lookup;

  enum class resolution : bool {
    delayed = false,
    immediate = true
  };

  struct def {
    label name; 
    term type;
    term value;

    def(label name, term type, term value)
      : name{name}, type{type}, value{value} { }
    
    def(label name, term value)
      : name{name}, type{value.sigma()->inferred_type()}, value{value} { }
    
    def(variable name, term type, term value) 
      : def(name.name(), type, value) { }
    
    def(variable name, term value)
      : def(name.name(), value) { }
  };

  struct function_def {
    label name;
    std::vector<decl> parameters;
    term range;
    term body;

    function_def(label name, std::vector<decl> parms, term range, term body)
      : name{name}, parameters{std::move(parms)}, range{range}, body{body} { }

    function_def(label name, std::vector<decl> parms, term body)
      : name{name}, 
        parameters{std::move(parms)}, 
        range{body.sigma()->inferred_type()}, 
        body{body} { }
    
    function_def(variable name, std::vector<decl> parms, term range, term body)
      : function_def(name.name(), std::move(parms), range, body) { }

    function_def(variable name, std::vector<decl> parms, term body)
      : function_def(name.name(), std::move(parms), body) { }
  };

  class module
  {
  public:
    explicit module(alphabet *sigma);
    module(module const&);
    module(module &&);

    ~module();
    
    module &operator=(module const&);
    module &operator=(module &&);

    bool operator==(module const&) const;
    
    //
    // Import other modules
    //
    void import(module m);
  
    //
    // declarations and definitions
    //
    object declare(decl d, resolution r = resolution::immediate);
    
    object define(def d, resolution r = resolution::immediate);
    object define(function_def f, resolution r = resolution::immediate);

    //
    // Name lookup
    //
    std::optional<object> lookup(label x) const;
    std::optional<object> lookup(variable x) const { return lookup(x.name()); }

    //
    // requirements
    //
    void require(term req);

    //
    // push()/pop() interface
    //
    void push();
    void pop();

    auto pushed(auto f) {
      auto _ = support::finally([&]{ pop(); });
      push();
      return f();
    }

    //
    // accessors
    //
    alphabet *sigma() const;
    std::vector<module> imports() const; // TODO: replace vector with lazy range
    std::vector<object> objects() const; // TODO: same as above
    std::vector<term> requirements() const; // TODO: same as above

    //
    // Resolve terms
    //
    term resolved(term t) const;

    //
    // Fully resolve the whole module
    //
    void resolve();
    module resolved() const;

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

  struct lookup {
    label name;
    term type;
    std::optional<term> value;

    explicit lookup(decl d) : name{d.name}, type{d.type} { }
    
    explicit lookup(def d) : name{d.name}, type{d.type}, value{d.value} { }

    bool operator==(lookup const&) const = default;
  };

}

#endif // BLACK_LOGIC_SCOPE_HPP
