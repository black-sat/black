//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#ifndef BLACK_SUPPORT_RESULT_HPP
#define BLACK_SUPPORT_RESULT_HPP

#include <type_traits>
#include <variant>

namespace black::support::internal {

  //
  // Utility for the next type below
  //
  template<typename T>
  struct alternatives : std::type_identity<std::tuple<>> { };

  template<matchable T>
  struct alternatives<T> : std::type_identity<typename T::alternatives> { };

  template<typename T>
  using alternatives_t = typename alternatives<T>::type;

  //
  // Expected-like result type for operations that may generate user-facing
  // errors (e.g. parsing, type checking, etc...).
  //
  
  template<typename Type, typename Error = error>
  class [[nodiscard("ignored result<T,E>, please check the error status")]] 
  result {
    using T = std::conditional_t<std::is_void_v<Type>, std::monostate, Type>;

  public:
    using alternatives = tuple_cons_t<T, alternatives_t<Error>>;

    result() = default;

    result(T&& v) : _data{std::forward<T>(v)} { }
    result(Error&& v) : _data{std::forward<Error>(v)} { }

    template<std::convertible_to<Error> E>
    result(E&& v) : _data{Error{std::forward<E>(v)}} { }

    result(result const &) = default;
    result(result &&) = default;
    
    result &operator=(result const &) = default;
    result &operator=(result &&) = default;

    bool operator==(result const&) const = default;

    //
    // Matching interface
    //
    template<typename To>
      requires matchable<Error>
    std::optional<To> to() const {
      if(variant_is<To>(_data))
        return variant_get<To>(_data);
      if(variant_is<Error>(_data)) {
        Error err = *variant_get<Error>(_data);
        return err.template to<To>();
      }
      return {};
    }
    
    template<typename To>
    std::optional<To> to() const {
      if(variant_is<To>(_data))
        return variant_get<To>(_data);
      return {};
    }

    template<typename To>
    bool is() const {
      return to<To>().has_value();
    }

    template<typename ...Handlers>
    auto match(Handlers ...h) {
      return matcher<result>::match(*this, h...);
    }

    //
    // expected-like interface
    //
    bool has_value() const {
      return variant_is<T>(_data);
    }

    T const&value() const {
      if(!has_value())
        throw bad_result_access(error());
      return *std::get<T>(_data);
    }
    
    T &value() {
      if(!has_value())
        throw bad_result_access(error());
      return *std::get<T>(_data);
    }

    T const&operator*() const {
      black_assert(has_value());
      return *std::get<T>(_data);
    }
    
    T &operator*() {
      black_assert(has_value());
      return *std::get<T>(_data);
    }

    T const*operator->() const {
      return &(*std::get<T>(_data));
    }

    T *operator->() {
      return &(*std::get<T>(_data));
    }

    template<std::convertible_to<T> U>
    T value_or(U&& else_) {
      if(has_value())
        return value();
      return std::forward<U>(else_);
    }

    Error const&error(source_location loc = source_location::current()) const {
      black_assume(!has_value(), loc, "missing error value");
      return std::get<Error>(_data);
    }
    
    Error &error(source_location loc = source_location::current()) {
      black_assume(!has_value(), loc, "missing error value");
      return std::get<Error>(_data);
    }

  private:
    std::variant<T, Error> _data;
  };
}

namespace black::support {
  using internal::syntax_error;
  using internal::type_error;
  using internal::backend_error;
  using internal::io_error;
  using internal::error;
  using internal::result;
}

#endif // BLACK_SUPPORT_RESULT_HPP
