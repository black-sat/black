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
#include <variant>
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

  class module;

  template<typename T>
  concept replay_target = requires(T v, object obj, module m, term t) {
    v.import(m);
    v.adopt(obj);
    v.require(t);
    v.push();
    v.pop(42);
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

    // adopt an existing object (maybe declared in another module)
    void adopt(object);

    // adopt a set of possibly recursively interconnected existing objects
    void adopt(std::vector<object> const& objs);

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
    // Comparisons
    //

    //
    // push()/pop() interface
    //
    // We need ad-hoc data structures to support both lookups and efficient
    // replay of stack history.
    // - we store the stack frames explicitly instead of copying the whole 
    //   module.
    //   * this implies changing the lookup logic but it will not affect 
    //     semantics if we traverse the stack from top to bottom for lookups
    // - now we can:
    //   * efficiently find a common prefix of the stack history of two modules
    //   * replay the stack history from a given prefix onwards
    // - this should be presented as a general way to iterate through the module
    // - we should also take this opportunity to account for groups of mutually 
    //   recursive declarations
    //
    void push();
    void pop(size_t n = 1);

    auto pushed(auto f) {
      auto _ = support::finally([&]{ pop(); });
      push();
      return f();
    }

    //
    // Issues a sequence of calls to member functions of `target` for each 
    // difference of `*this` and `from`.
    //
    // The calls are arranged in such a way that after the following call:
    //
    //    m.replay(from, from);
    // 
    // then `m == from` is guaranteed to hold.
    //
    template<replay_target T>
    void replay(module const& from, T &target) const;

    //
    // accessors
    //
    alphabet *sigma() const;

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

    struct replay_target_t {
      virtual ~replay_target_t() = default;
      virtual void import(module) = 0;
      virtual void adopt(object) = 0;
      virtual void require(term) = 0;
      virtual void push() = 0;
      virtual void pop(size_t) = 0;
    };

    void replay(module const& from, replay_target_t *target) const;

    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

  struct lookup : std::enable_shared_from_this<lookup> {
    label name;
    term type;
    std::optional<term> value;

    explicit lookup(decl d) : name{d.name}, type{d.type} { }
    
    explicit lookup(def d) : name{d.name}, type{d.type}, value{d.value} { }

    bool operator==(lookup const& o) const {
      return name == o.name && type == o.type && value == o.value;
    }
  };

  //
  // Implementation of module::replay interface
  //
  template<replay_target T>
  void module::replay(module const &from, T &target) const {
    struct wrap_t : replay_target_t {
      T *t;

      wrap_t(T *t) : t{t} { }

      virtual void import(module m) override { return t->import(std::move(m)); }
      virtual void adopt(object obj) override { return t->adopt(obj); }
      virtual void require(term r) override { return t->require(r); }
      virtual void push() override { t->push(); }
      virtual void pop(size_t n) override { t->pop(n); }

    } wrap{&target};

    return replay(from, &wrap);
  }

}

#endif // BLACK_LOGIC_SCOPE_HPP
