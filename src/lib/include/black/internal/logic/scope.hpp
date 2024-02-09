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

    struct lookup {
      symbol name;
      term result;
      scope const *origin;
    };

    scope() = default;
    scope(scope const&) = delete;
    scope(scope &&) = delete;

    virtual ~scope() = default;
    
    scope &operator=(scope const&) = delete;
    scope &operator=(scope &&) = delete;

    virtual alphabet *sigma() const = 0;

    virtual std::optional<lookup> decl_of(symbol s) const = 0;
    virtual std::optional<lookup> def_of(symbol s) const = 0;

    result<term> type_of(term t) const;
    result<term> value_of(term t) const;

    result<bool> is_type(term t) const;

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

    virtual std::optional<lookup> decl_of(symbol s) const override;
    virtual std::optional<lookup> def_of(symbol s) const override;

    alphabet *sigma() const override;
    scope const *base() const;

    result<void> declare(symbol s, term type);
    result<void> define(symbol s, term value);
    
    result<void> declare(decl d);
    result<void> define(def d);
    result<void> declare(std::vector<decl> const& decls);
    result<void> define(std::vector<def> const& defs);
    result<void> declare(symbol s, std::vector<term> params, term range);
    result<void> define(symbol s, std::vector<decl> params, term body);

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };
}

#endif // BLACK_LOGIC_SCOPE_HPP
