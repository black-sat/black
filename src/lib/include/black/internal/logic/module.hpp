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

  class module
  {
  public:
    explicit module(alphabet *sigma);
    module(module const&) = delete;
    module(module &&) = delete;
    
    module &operator=(module const&) = delete;
    module &operator=(module &&) = delete;
    
    //
    // Import other modules
    //
    void import(module const *);

    //
    // Declarations and definitions
    //
    object declare(label s, term type);
    object declare(binding d);
    object declare(label s, std::vector<term> params, term range);
    std::vector<object> declare(std::vector<binding> const& binds);
    
    object define(label s, term type, term def);
    object define(def d);
    object define(label s, std::vector<binding> params, term range, term body);
    std::vector<object> define(std::vector<def> const& defs);

    //
    // accessors
    //
    alphabet *sigma() const;

    using decl_range_t = 
      std::ranges::transform_view<
        std::ranges::ref_view<support::map<label, std::shared_ptr<decl>>>, 
        std::shared_ptr<decl> std::pair<label, std::shared_ptr<decl>>::*
      >;

    decl_range_t declarations() const;


    //
    // Lookup of single symbols
    //
    std::shared_ptr<decl const> lookup(label s) const;

    //
    // Resolution of terms
    //
    term resolve(term t, support::set<variable> const& shadow = {}) const;
    void resolve();

    //
    // Type checking and term evaluation
    //
    term type_of(term t) const;
    term evaluate(term t) const;

    std::vector<term> type_of(std::vector<term> const& ts) const;
    std::vector<term> evaluate(std::vector<term> const& ts) const;

  private:
    struct _impl_t;
    
    module(std::shared_ptr<_impl_t>);
    std::shared_ptr<_impl_t> _impl;
  };
}

#endif // BLACK_LOGIC_SCOPE_HPP
