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
  // Boxes an instance of type T created on demand, to be used in pimpl
  // patterns.
  //
  template<typename T>
  class boxed 
  {
  public:
    boxed() = default;

    boxed(std::unique_ptr<T> ptr) : _ptr{std::move(ptr)} { }
    
    boxed(boxed const& other) 
      : _ptr{other._ptr ? std::make_unique<T>(*other._ptr) : nullptr} { }

    boxed(boxed &&) = default;

    boxed &operator=(boxed const& other) {
      _ptr = other._ptr ? std::make_unique<T>(*other._ptr) : nullptr;
      return *this;
    }

    boxed &operator=(boxed &&) = default;

    bool operator==(boxed const&other) const {
      return get() == other.get() || *get() == *other.get();
    }

    explicit operator bool() const { return true; }

    T *get() const {
      if(!_ptr)
        _ptr = std::make_unique<T>();
      return _ptr.get();
    }

    std::unique_ptr<T> release() && {
      return std::move(_ptr);
    }

    T *operator->() const {
      return get();
    }

  private:
    mutable std::unique_ptr<T> _ptr = nullptr;
  };

}

#endif // BLACK_SUPPORT_MEMORY_HPP
