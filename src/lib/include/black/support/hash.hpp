//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#ifndef BLACK_SUPPORT_HASH_HPP
#define BLACK_SUPPORT_HASH_HPP

#include <black/support/meta.hpp>
#include <black/support/assert.hpp>
#include <black/support/to_string.hpp>

#include <any>
#include <tuple>
#include <optional>
#include <vector>
#include <typeinfo>

//
// std::hash specialization for tuples, pairs, and vectors
// See https://stackoverflow.com/a/27952689/3206471
// and https://stackoverflow.com/questions/35985960
// for an explanation of the hashing function combination technique
//
namespace black::internal {
  inline size_t hash_combine(size_t lhs, size_t rhs) {
    static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);

    if constexpr(sizeof(size_t) == 4) {
      lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    } else if constexpr(sizeof(size_t) == 8) {
      lhs ^= rhs + 0x9e3779b97f4a7c15 + (lhs << 6) + (lhs >> 2);
    }

    return lhs;
  }
}

// NB: Declaring things in namespace std is allowed for specialization of
// standard templates
namespace std
{
  template<typename T>
  struct hash<tuple<T>> {
    size_t operator()(tuple<T> const&t) const {
      return hash<T>{}(get<0>(t));
    }
  };

  template<typename T, typename ...Ts>
  struct hash<tuple<T,Ts...>> {
    size_t operator()(tuple<T,Ts...> const&t) const {
      using namespace ::black::internal;

      hash<T> h1;
      hash<tuple<Ts...>> h2;

      return hash_combine(h1(std::get<0>(t)), h2(tuple_tail(t)));
    }
  };

  template<typename S, typename T>
  struct hash<pair<S, T>>
  {
    size_t operator()(pair<S, T> const&v) const {
      return hash<tuple<S,T>>{}(make_tuple(v.first,v.second));
    }
  };
}

namespace black::internal
{
  //
  // Trait to detect whether a type is convertible to a tuple of some arbitrary
  // types
  //
  template<typename T>
  struct is_tuple_t : std::false_type {};

  template<typename ...Args>
  struct is_tuple_t<std::tuple<Args...>> : std::true_type {};

  template<typename T>
  constexpr bool is_tuple = is_tuple_t<std::decay_t<T>>::value;

  //
  // Check if a type is hashable
  // https://stackoverflow.com/questions/12753997
  //
  // TODO: check for proper enabled specializations
  //
  template <typename T, typename = void>
  struct is_hashable_t : std::false_type { };

  template <typename T>
  struct is_hashable_t<T,
    std::void_t<
      decltype(std::declval<std::hash<std::decay_t<T>>>()(std::declval<T>()))
    >
  > : std::true_type { };

  template <typename T>
  constexpr bool is_hashable = is_hashable_t<T>::value;

  //
  // Type-erased hashable value
  //
  class identifier
  {
  public:
    identifier() = default;
    identifier(identifier const&) = default;
    identifier(identifier&&) = default;

    template<
      typename T,
      REQUIRES(!std::is_convertible_v<T, identifier>),
      REQUIRES(is_hashable<std::decay_t<T>>)
    >
    explicit identifier(T&& value)
      : _any(FWD(value)),
        _hash(make_hasher(value)),
        _cmp(make_cmp(value)),
        _printer(make_printer(value)) {}

    size_t hash() const {
      black_assert(_any.has_value());
      return _hash(_any);
    }

    identifier &operator=(identifier const&) = default;
    identifier &operator=(identifier&&) = default;

    bool operator==(identifier const&other) const {
      return _cmp(_any, other);
    }

    bool operator!=(identifier const&other) const {
      return !_cmp(_any, other);
    }

    template<typename T, REQUIRES(is_hashable<std::decay_t<T>>)>
    identifier &operator=(T&& value) {
      _any = FWD(value);
      _hash = make_hasher(value);
      _cmp = make_cmp(value);
      _printer = make_printer(value);
      return *this;
    }

    template<typename T>
    bool is() const {
      return std::any_cast<T>(&_any) != nullptr;
    }

    template<typename T>
    std::optional<T> to() const & {
      if(T const*ptr = std::any_cast<T>(&_any); ptr)
        return std::optional<T>{*ptr};
      return std::nullopt;
    }

    template<typename T>
    std::optional<T> to() && {
      if(T const*ptr = std::any_cast<T>(&_any); ptr)
        return std::optional<T>{std::move(*ptr)};
      return std::nullopt;
    }

    template<typename T>
    T const* get() const & { return std::any_cast<T>(&_any); }

    template<typename T>
    T *get() & { return std::any_cast<T>(&_any); }

    std::any const&any() const { return _any; }

    friend std::string to_string(identifier const&id) {
      return id._printer(id._any);
    }

  private:
    using hasher_t = size_t (*)(std::any const&);
    using comparator_t = bool (*)(std::any const&, identifier const&);
    using printer_t = std::string (*)(std::any const&);

    std::any _any;
    hasher_t _hash;
    comparator_t _cmp;
    printer_t _printer;

    //
    // note: these two function templates cause gcov false negatives
    template<typename T>
    hasher_t make_hasher(T const&) { // LCOV_EXCL_LINE
      return [](std::any const&me) -> size_t { // LCOV_EXCL_LINE
        T const *v = std::any_cast<T>(&me); // LCOV_EXCL_LINE
        black_assert(v != nullptr); // LCOV_EXCL_LINE

        return std::hash<T>{}(*v); // LCOV_EXCL_LINE
      };
    }

    template<typename T>
    comparator_t make_cmp(T const&) { // LCOV_EXCL_LINE
      return [](std::any const&me, identifier const&other) -> bool { // LCOV_EXCL_LINE
        T const* v = std::any_cast<T>(&me); // LCOV_EXCL_LINE
        T const* otherv = other.get<T>(); // LCOV_EXCL_LINE

        black_assert(v != nullptr); // LCOV_EXCL_LINE

        return otherv != nullptr && *v == *otherv; // LCOV_EXCL_LINE
      };
    }

    template<typename T, REQUIRES(is_stringable<T>)>
    printer_t make_printer(T const&) {
      return [](std::any const&me) -> std::string {
        T const *v = std::any_cast<T>(&me);
        black_assert(v != nullptr);

        return to_string(*v);
      };
    }
    
    template<typename T, REQUIRES(!is_stringable<T>)>
    printer_t make_printer(T const&) {
      return [](std::any const&) -> std::string {
        return "<" + std::string{typeid(T).name()} + ">";
      };
    }
  };
}

namespace black {
  using internal::identifier;
}

// std::hash specialization for identifier
namespace std {
  template<>
  struct hash<black::internal::identifier> {
    size_t operator()(black::internal::identifier const&h) const {
      return h.hash();
    }
  };
}

#endif
