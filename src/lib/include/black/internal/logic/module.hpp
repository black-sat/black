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
    variable name; 
    term type;
    term value;

    def(variable name, term type, term value) 
      : name{name.name()}, type{type}, value{value} { }
    
    def(variable name, term value)
      : def{name, inferred_type(), value} { }
  };

  struct function_def {
    variable name;
    std::vector<decl> parameters;
    term range;
    term body;

    function_def(variable name, std::vector<decl> parms, term range, term body)
      : name{name}, parameters{std::move(parms)}, range{range}, body{body} { }

    function_def(variable name, std::vector<decl> parms, term body)
      : name{name}, 
        parameters{std::move(parms)}, 
        range{inferred_type()}, 
        body{body} { }
  };

  class module;

  enum class scope {
    recursive,
    linear
  };

  template<typename T>
  concept replay_target = requires(T v, object obj, module m, term t) {
    v.import(m);
    v.adopt(std::vector<object>{}, scope::recursive);
    v.require(t);
    v.push();
    v.pop(42);
  };

  class module
  {
  public:
    module();
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

    // adopt a set of existing possibly recursive objects (declared or defined)
    void adopt(std::vector<object> const& objs, scope s = scope::recursive);

    //
    // Name lookup
    //
    std::optional<object> lookup(variable x) const;

    //
    // requirements
    //
    void require(term req);

    //
    // push()/pop() interface
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
    // Resolve terms
    //
    term resolved(term t) const;

    //
    // Fully resolve the whole module
    //
    void resolve(scope s = scope::linear);
    module resolved(scope s = scope::linear) const;

  private:

    struct replay_target_t {
      virtual ~replay_target_t() = default;
      virtual void import(module) = 0;
      virtual void adopt(std::vector<object> const&, scope s) = 0;
      virtual void require(term) = 0;
      virtual void push() = 0;
      virtual void pop(size_t) = 0;
    };

    void replay(module const& from, replay_target_t *target) const;

    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

  struct lookup : std::enable_shared_from_this<lookup> {
    variable name;
    term type;
    std::optional<term> value;

    explicit lookup(decl d) : name{d.name}, type{d.type} { }
    
    explicit lookup(def d) : name{d.name}, type{d.type}, value{d.value} { }
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
      virtual void adopt(std::vector<object> const& objs, scope s) override { 
        return t->adopt(objs, s); 
      }
      virtual void require(term r) override { return t->require(r); }
      virtual void push() override { t->push(); }
      virtual void pop(size_t n) override { t->pop(n); }

    } wrap{&target};

    return replay(from, &wrap);
  }

}

#endif // BLACK_LOGIC_SCOPE_HPP