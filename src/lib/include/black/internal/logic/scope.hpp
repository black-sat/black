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


  //
  // We want to make sure that term AST nodes are uniqued only when 
  // semantically they are indeed the same thing, that is:
  //  ** two symbols are uniqued only if they resolve to the same **
  //  ** declaration/definition inside the same scope             **
  // 
  // This can be done in two ways:
  // 1. the scope is a parameter of the symbol object
  // 2. the declaration is an object with its own identity and it's a parameter
  //    of the symbol object
  //

  class decl 
  {
  protected:
    struct ctor_t { };
  public:
    decl(decl const&) = delete;

    decl &operator=(decl const&) = delete;

    symbol name() const { return _name; }
    term type() const { return _type; }
    std::optional<term> def() const { return _def; }

    void def(term t) { _def = t; }

    decl(ctor_t, symbol name, term type, std::optional<term> def)
      : _name{name}, _type{type}, _def{def} { }


  private:
    friend class scope;
    
    symbol _name;
    term _type;
    std::optional<term> _def;
  };

  class scope 
  {
  public:
    struct error : support::error { 
      using support::error::error;

      template<typename T>
      operator std::expected<T, error>() const {
        return std::unexpected(*this);
      }
    };
    
    template<typename T>
    using result = std::expected<T, error>;

    scope() = default;
    scope(scope const&) = delete;
    scope(scope &&) = delete;

    virtual ~scope() = default;
    
    scope &operator=(scope const&) = delete;
    scope &operator=(scope &&) = delete;

    virtual alphabet *sigma() const = 0;

    virtual std::optional<decl const *> lookup(symbol s) const = 0;

    result<term> resolve(term t, support::set<symbol> const& shadow = {}) const;

    result<term> type_of(term t) const;
    result<term> evaluate(term t) const;

    result<bool> is_type(term t) const;

  protected:
    static std::shared_ptr<decl> 
    make_decl(symbol name, term type, std::optional<term> def) {
      return std::make_shared<decl>(decl::ctor_t{}, name, type, def);
    }

  };

  class module : public scope 
  {
  public:
    module(alphabet *sigma);
    module(scope const* sigma);
    module(module const&) = delete;
    module(module &&) = delete;

    virtual ~module() override;
    
    module &operator=(module const&) = delete;
    module &operator=(module &&) = delete;


    alphabet *sigma() const override;
    scope const *base() const;

    result<void> declare(symbol s, term type);
    result<void> define(symbol s, term value);
    
    result<void> declare(binding d);
    result<void> define(binding d);
    result<void> declare(std::vector<binding> const& binds);
    result<void> define(std::vector<binding> const& binds);
    result<void> declare(symbol s, std::vector<term> params, term range);
    result<void> define(symbol s, std::vector<binding> params, term body);
    
  protected:
    virtual std::optional<decl const *> lookup(symbol s) const override;

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };
}

#endif // BLACK_LOGIC_SCOPE_HPP
