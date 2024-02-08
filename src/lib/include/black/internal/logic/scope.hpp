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

  struct type_error : support::error { 
    using error::error;

    template<typename T>
    operator std::expected<T, type_error>() const {
      return std::unexpected(*this);
    }
  };
  
  template<typename T>
  using type_result = std::expected<T, type_error>;

  class scope 
  {
  public:
    scope() = default;
    scope(scope const&) = delete;
    scope(scope &&) = delete;

    virtual ~scope() = default;
    
    scope &operator=(scope const&) = delete;
    scope &operator=(scope &&) = delete;

    virtual type_result<term> type_of(symbol s) const = 0;
    virtual type_result<term> definition_of(symbol s) const = 0;

    type_result<term> type_of(term t) const;
    type_result<term> definition_of(term t) const;
    
    type_result<std::vector<term>> 
      type_of(std::vector<term> const& t) const;
    type_result<std::vector<term>> 
      definition_of(std::vector<term> const& t) const;

  };

  class module : public scope 
  {
  public:
    module(alphabet *sigma);
    module(module const&) = delete;
    module(module &&) = delete;

    virtual ~module() override;
    
    module &operator=(module const&) = delete;
    module &operator=(module &&) = delete;

    alphabet *sigma() const;

    virtual type_result<term> type_of(symbol s) const override;
    virtual type_result<term> definition_of(symbol s) const override;

    type_result<void> declare(symbol s, term type);
    type_result<void> define(symbol s, term value);
    type_result<void> declare(symbol s, std::vector<term> params, term range);
    type_result<void> define(symbol s, std::vector<decl> params, term body);

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

}

#endif // BLACK_LOGIC_SCOPE_HPP
