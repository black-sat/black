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

  class module;

  struct decl { 
    module const *scope;
    variable name; 
    term type;
    std::optional<term> def;
  };

  struct def {
    variable name; 
    term type;
    term def;
  };

  class cache 
  {
  public:
    cache();
    cache(cache const&);
    cache(cache &&);

    ~cache();
    
    cache &operator=(cache const&);
    cache &operator=(cache &&);
    
    std::optional<std::any> get(term k) const;
    void insert(term k, std::any v);
    void clear();

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

  class module
  {
  public:
    explicit module(alphabet *sigma);
    module(module const&);
    module(module &&) = default;

    ~module();
    
    module &operator=(module const&);
    module &operator=(module &&) = default;
    
    //
    // Import other modules
    //
    void import(module m);

    //
    // Declarations and definitions
    //
    variable declare(variable s, term type);
    variable declare(label s, term type);
    variable declare(binding d);
    variable declare(variable s, std::vector<term> const &params, term range);
    variable declare(label s, std::vector<term> const &params, term range);
    void declare(std::vector<binding> const& binds);
    
    variable define(variable s, term type, term def);
    variable define(label s, term type, term def);
    variable define(def d);
    variable define(
      variable s, std::vector<binding> const &params, term range, term body
    );
    variable define(
      label s, std::vector<binding> const &params, term range, term body
    );
    void define(std::vector<def> const& defs);

    void undef(variable x);
    void undef(label s);
    void undef(std::vector<variable> const& vars);

    //
    // accessors
    //
    alphabet *sigma() const;

    //
    // Lookup of variables
    //
    std::optional<decl> lookup(variable s) const;

    //
    // Generic term cache for the users of this module
    //
    class cache &cache();

    class cache const& cache() const;

    //
    // Type checking and term evaluation
    //
    term type_of(term t) const;
    term evaluate(term t) const;

    std::vector<term> type_of(std::vector<term> const& ts) const;
    std::vector<term> evaluate(std::vector<term> const& ts) const;

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };
}

#endif // BLACK_LOGIC_SCOPE_HPP
