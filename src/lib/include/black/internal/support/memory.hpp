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

#ifndef BLACK_SUPPORT_MEMORY_HPP
#define BLACK_SUPPORT_MEMORY_HPP

#include <memory>
#include <variant>

namespace black::support {

  //
  // a reference to an object managed through `std::shared_ptr`,
  // forming either a strong or a weak reference, on demand.
  //
  // This allows it to be initialized as a strong or weak reference
  // depending on the case, while still be hashable (which a `weak_ptr` is not).
  //
  template<typename T>
  class wrap_ptr 
  {
  public:
    wrap_ptr() = delete;
    wrap_ptr(nullptr_t) = delete;

    wrap_ptr(std::shared_ptr<T> ptr) : _data{std::move(ptr)} { }

    explicit wrap_ptr(T *ptr) : _data{ptr} { 
      black_assert(ptr);
    }

    wrap_ptr(wrap_ptr const&) = default;
    wrap_ptr(wrap_ptr &&) = default;
    
    wrap_ptr &operator=(wrap_ptr const&) = default;
    wrap_ptr &operator=(wrap_ptr &&) = default;

    wrap_ptr &operator=(std::shared_ptr<T> ptr) { 
      _data = std::move(ptr);
      return *this;
    }

    wrap_ptr &operator=(T *ptr) { 
      black_assert(ptr);
      _data = ptr;
      return *this;
    }

    wrap_ptr &operator=(nullptr_t) = delete;

    bool operator==(wrap_ptr const&) const = default;

    operator wrap_ptr<T const>() const
      requires (!std::is_const_v<T>)
    { 
      return match(_data)(
        [](std::shared_ptr<T> p) { return wrap_ptr<T const>(p); },
        [](T *p) { return wrap_ptr<T const>(p); }
      );
    }

    T *get() const {
      return match(_data)(
        [](std::shared_ptr<T> p) { return p.get(); },
        [](T *p) { return p; }
      );
    }

    std::shared_ptr<T> lock() const {
      return match(_data)(
        [](std::shared_ptr<T> p) { return p; },
        [](T *p) { return p->shared_from_this(); }
      );
    }

    wrap_ptr<T> unlocked() const {
      return {
        match(_data)(
          [](std::shared_ptr<T> p) { return p.get(); },
          [](T *p) { return p; }
        )
      };
    }

    auto operator*() const {
      struct ret_t {
        std::shared_ptr<T> locked;

        T &operator=(T const& other) const 
          requires (!std::is_const_v<T>) 
        {
          *locked = other;
          return *locked;
        }
        
        T &operator=(T && other) const 
          requires (!std::is_const_v<T>) 
        {
          *locked = other;
          return *locked;
        }

        operator T&() const {
          return *locked;
        }
      } ret_t{lock()}; 

      return ret_t;
    }

    std::shared_ptr<T> operator->() const {
      return lock();
    }

    size_t hash() const {
      return match(_data)(
        [](std::shared_ptr<T> p) { return support::hash(p); },
        [](T *p) { return support::hash(p); }
      );
    }

  private:
    std::variant<std::shared_ptr<T>, T *> _data;
  };
  
}

#endif // BLACK_SUPPORT_MEMORY_HPP
