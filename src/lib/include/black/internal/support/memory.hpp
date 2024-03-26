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
#include <any>
#include <concepts>

namespace black::support {

  //
  // Boxes an instance, managing it on the heap but keeping value semantics.
  //
  template<typename T>
  class boxed 
  {
  public:
    boxed() : _ptr{std::make_unique<T>()} { }
    
    boxed(T v) : _ptr{std::make_unique<T>(std::move(v))} { }

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

    bool empty() const { return _ptr; }

    T const* get() const {
      return _ptr.get();
    }
    
    T *get() {
      return _ptr.get();
    }

    std::unique_ptr<T> release() {
      return std::move(_ptr);
    }

    T const* operator->() const {
      return get();
    }
    
    T *operator->() {
      return get();
    }

  private:
    std::unique_ptr<T> _ptr;
  };

  //
  // Manages an instance of type T which is, however, only constructed at first
  // access.
  //
  template<typename T>
  class lazy 
  {
  public:
    lazy() = default;
    lazy(T v) : _data{std::move(v)} { }
    
    lazy(lazy const&) = default;
    lazy(lazy &&) = default;
    
    lazy &operator=(T v) {
      _data = std::move(v);
      return *this;
    }

    lazy &operator=(lazy const&) = default;
    lazy &operator=(lazy &&) = default;

    bool operator==(lazy const&) const = default;

    T const*get() const {
      if(!_data)
        _data = T{};
      return &(*_data);
    }
    
    T *get() {
      if(!_data)
        _data = T{};
      return &(*_data);
    }

    T const&operator*() const {
      return *get();
    }
    
    T &operator*() {
      return *get();
    }

    T const*operator->() const {
      return get();
    }
    
    T *operator->() {
      return get();
    }
  
  private:
    mutable std::optional<T> _data;
  };

  //
  // Similar to `std::any` but knows the base class of the contained type and
  // can compare two values of the same type. That is, value semantics and
  // virtual dispatch together.
  //
  template<typename Base>
  class any 
  {
  public:
    any() = default;

    template<std::equality_comparable Derived>
      requires std::is_base_of_v<Base, Derived>
    any(Derived v)
      : _value{std::move(v)}, 
        _extractor{make_extractor<Derived>()},
        _comparator{make_comparator<Derived>()} { }

    template<typename Derived>
      requires std::is_base_of_v<Base, Derived>
    any(any<Derived> v)
      : _value{std::move(v._value)},
         _extractor{make_extractor<Derived>(v._extractor)}, 
         _comparator{v._comparator} { }

    any(any const&) = default;
    any(any &&) = default;
    
    any &operator=(any const&) = default;
    any &operator=(any &&) = default;

    template<std::equality_comparable Derived>
      requires std::is_base_of_v<Base, Derived>
    any &operator=(Derived v) {
      *this = any{std::move(v)};
      return *this;
    }

    template<typename Derived>
      requires std::is_base_of_v<Base, Derived>
    any &operator=(any<Derived> v) {
      *this = any{std::move(v)};
      return *this;
    }

    bool operator==(any const& other) const {
      if(!_value.has_value() || !other._value.has_value())
        return !_value.has_value() && !other._value.has_value();

      return _comparator(&_value, &other._value);
    }

    bool has_value() const {
      return _value.has_value();
    }

    Base *get() {
      return _extractor(&_value);
    }
    
    Base const *get() const {
      return _extractor(&_value);
    }

    Base *operator->() {
      return get();
    }

    Base const*operator->() const {
      return get();
    }

    template<typename Derived>
    friend Derived extract(any v) {
      return std::any_cast<Derived>(std::move(v._value));
    }

    template<typename Derived>
    friend Derived *extract(any *v) {
      return std::any_cast<Derived>(&v->_value);
    }
    
    template<typename Derived>
    friend Derived const *extract(any const*v) {
      return std::any_cast<Derived>(&v->_value);
    }
    
    

  private:
    template<typename U>
    friend class any;

    using extractor_t = std::function<std::remove_const_t<Base> *(std::any *)>;
    using comparator_t = std::function<bool(std::any const*, std::any const*)>;

    template<typename Derived>
    extractor_t make_extractor() {
      return [](std::any *value) -> std::remove_const_t<Base> * {
        return std::any_cast<Derived>(value);
      };
    }

    template<typename Derived>
    extractor_t make_extractor(any<Derived>::extractor_t ex) {
      return [=](std::any *value) -> Base * {
        return ex(value);
      };
    }

    template<typename Derived>
    comparator_t make_comparator() {
      return [](std::any const *v1, std::any const *v2) {
        if(!v1 || !v2)
          return !v1 && !v2;
        
        Derived const *p1 = std::any_cast<Derived>(v1);
        Derived const *p2 = std::any_cast<Derived>(v2);

        return p1 && p2 && *p1 == *p2;
      };
    }

    mutable std::any _value;
    extractor_t _extractor = nullptr;
    comparator_t _comparator = nullptr;
  };

  // //
  // // Variant of different handles for a pointer to type T
  // //
  // template<typename T>
  // class handle
  // {
  // private:
  //   std::variant<T *, std::shared_ptr<T>, 
  // };

}

#endif // BLACK_SUPPORT_MEMORY_HPP
