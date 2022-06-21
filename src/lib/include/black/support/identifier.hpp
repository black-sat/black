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

#include <black/support/assert.hpp>
#include <black/support/to_string.hpp>

#include <any>
#include <tuple>
#include <optional>
#include <vector>
#include <string_view>
#include <typeinfo>

//
// Function to combine different hashe values into one.
// See https://stackoverflow.com/a/27952689/3206471
// and https://stackoverflow.com/questions/35985960
// for an explanation of the algorithm
//
namespace black_internal {
  inline size_t hash_combine(size_t lhs, size_t rhs) {
    static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);

    if constexpr(sizeof(size_t) == 4) {
      lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    } else if constexpr(sizeof(size_t) == 8) {
      lhs ^= rhs + 0x9e3779b97f4a7c15 + (lhs << 6) + (lhs >> 2);
    }

    return lhs;
  }

  template<typename T>
  concept hashable = requires(T t1, T t2) {
    std::hash<std::remove_cvref_t<T>>{}(t1);
    t1 == t2;
  };
}

namespace black_internal::identifier_details
{
  //
  // Type-erased hashable value
  //
  class identifier
  {
  public:
    identifier() = default;
    identifier(identifier const&) = default;
    identifier(identifier&&) = default;

    template<hashable T>
      requires (!std::is_same_v<std::remove_cvref_t<T>, identifier>)
    identifier(T&& value)
      : _any(std::forward<T>(value)),
        _hash(make_hasher(value)),
        _cmp(make_cmp(value)),
        _printer(make_printer(value)) { }

    template<typename ...T>
    identifier(std::tuple<T...> const& t) 
      : _any(t),
        _hash(make_tuple_hasher(t)),
        _cmp(make_cmp(t)),
        _printer(make_printer(t)) { }

    template<typename T, typename U>
    identifier(std::pair<T, U> const& t) 
      : _any(t),
        _hash(make_pair_hasher(t)),
        _cmp(make_cmp(t)),
        _printer(make_printer(t)) { }

    identifier(std::string_view view) 
      : identifier{std::string{view}} { }

    identifier(char const* c_str) 
      : identifier{std::string{c_str}} { }

    size_t hash() const {
      black_assert(_any.has_value());
      return _hash(_any);
    }

    identifier &operator=(identifier const&) = default;
    identifier &operator=(identifier&&) = default;

    bool operator==(identifier const&other) const {
      return _cmp(_any, other);
    }

    template<hashable T>
    identifier &operator=(T&& value) {
      _any = std::forward<T>(value);
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
    
    template<typename ...T>
    hasher_t make_tuple_hasher(std::tuple<T...> const&) {
      return [](std::any const&me) -> size_t {
        std::tuple<T...> const *t = std::any_cast<std::tuple<T...>>(&me);
        black_assert(t != nullptr);

        return std::apply([](auto ...v) {
          size_t h = 0;
          ((h = hash_combine(h, std::hash<decltype(v)>{}(v))), ...);
          return h;
        }, *t);
      };
    }
    
    template<typename T, typename U>
    hasher_t make_pair_hasher(std::pair<T, U> const&) {
      return [](std::any const&me) -> size_t {
        std::pair<T, U> const *p = std::any_cast<std::pair<T, U>>(&me);
        black_assert(p != nullptr);

        size_t h1 = std::hash<T>{}(p->first);
        size_t h2 = std::hash<U>{}(p->second);
        return hash_combine(h1, h2);
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

    template<stringable T>
    printer_t make_printer(T const&) {
      return [](std::any const&me) -> std::string {
        T const *v = std::any_cast<T>(&me);
        black_assert(v != nullptr);

        return to_string(*v);
      };
    }
    
    template<typename T>
    printer_t make_printer(T const&) {
      return [](std::any const&) -> std::string {
        return std::string{typeid(T).name()};
      };
    }
  };
}

namespace black_internal {
  using identifier_details::identifier;
}
namespace black {
  using black_internal::identifier;
}

// std::hash specialization for identifier
namespace std {
  template<>
  struct hash<black::identifier> {
    size_t operator()(black::identifier const&h) const {
      return h.hash();
    }
  };
}

#endif
