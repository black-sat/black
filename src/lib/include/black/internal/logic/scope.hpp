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

  struct decl { 
    label name; 
    term type;
    std::optional<term> def;
  };

  struct def {
    label name; 
    term type;
    term def;
  };

  class scope 
  {
  public:
    enum class depth {
      deep, shallow
    };

    scope() = default;
    scope(scope const&) = delete;
    scope(scope &&) = delete;

    virtual ~scope() = default;
    
    scope &operator=(scope const&) = delete;
    scope &operator=(scope &&) = delete;

    virtual alphabet *sigma() const = 0;

    virtual std::shared_ptr<decl const> lookup(label s, depth d) const = 0;

    term resolve(term t, depth d, support::set<symbol> const& shadow = {})const;

    term type_of(term t) const;
    term evaluate(term t) const;

    std::vector<term> type_of(std::vector<term> const& ts) const;
    std::vector<term> evaluate(std::vector<term> const& ts) const;

    bool is_type(term t) const;

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

    virtual std::shared_ptr<decl const> lookup(label s, depth d) const override;

    alphabet *sigma() const override;
    scope const *base() const;

    variable declare(label s, term type);
    variable declare(binding d);
    variable declare(label s, std::vector<term> params, term range);
    std::vector<variable> declare(std::vector<binding> const& binds);
    
    variable define(label s, term type, term def);
    variable define(def d);
    variable 
      define(symbol s, std::vector<binding> params, term range, term body);
    variable 
      define(label s, std::vector<binding> params, term range, term body);
    std::vector<variable> define(std::vector<def> const& defs);

    using scope::resolve;
    void resolve();

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };
}

#endif // BLACK_LOGIC_SCOPE_HPP
