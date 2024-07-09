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

#ifndef BLACK_LOGIC_MODELS_HPP
#define BLACK_LOGIC_MODELS_HPP

#include <black/logic>

#include <limits>

namespace black::logic {

  class model 
  {
  public:
    class base;
    using ptr = std::shared_ptr<const base>;
    
    static constexpr size_t infinite = std::numeric_limits<size_t>::max();

    model();
    model(ptr p);
    model(model const&) = default;
    model(model &&) = default;
    
    model &operator=(model const&) = default;
    model &operator=(model &&) = default;

    std::optional<term> value(object x) const;

    std::optional<term> value(object x, size_t t) const;

    size_t size() const;

  private:
    ptr _ptr;
  };

  class model::base 
  {
  public:
    base() = default;
    virtual ~base() = default;

    virtual std::optional<term> value(object x) const = 0;

    virtual std::optional<term> value(object x, size_t t) const = 0;

    virtual size_t size() const = 0;
  };

  class empty_t : public model::base 
  {
  public:
    empty_t() = default;
    virtual ~empty_t() override = default;
    
    virtual std::optional<term> value(object) const override { return {}; }

    virtual std::optional<term> value(object, size_t) const override { 
      return {}; 
    }

    virtual size_t size() const override { return 0; }
  };

  inline model::model() : _ptr{std::make_shared<empty_t>()} { }

  inline model::model(ptr p) : _ptr{p} { }

  inline std::optional<term> model::value(object x) const {
    return _ptr->value(x);
  }

  inline std::optional<term> model::value(object x, size_t t) const {
    return _ptr->value(x, t);
  }

  inline size_t model::size() const {
    return _ptr->size();
  }

}

#endif // BLACK_LOGIC_MODELS_HPP
