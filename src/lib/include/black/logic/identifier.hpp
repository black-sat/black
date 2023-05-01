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

#ifndef BLACK_SUPPORT_IDENTIFIER_HPP
#define BLACK_SUPPORT_IDENTIFIER_HPP

#include <black/support/hash.hpp>

#include <fmt/format.h>

#include <any>
#include <tuple>
#include <optional>
#include <vector>
#include <string_view>
#include <string>

namespace black::logic::internal
{
  class identifier;

  template<typename T>
  concept formattable = std::is_constructible<fmt::formatter<T>>::value;

  template<typename T>
  concept identifiable = 
    !std::is_same_v<std::remove_cvref_t<T>, identifier> &&
    support::hashable<T> && std::equality_comparable<T> && formattable<T>;

  //
  // Type-erased hashable, comparable and printable value
  //
  class identifier
  {
  public:
    identifier() = default;
    identifier(identifier const&) = default;
    identifier(identifier&&) = default;

    template<identifiable T>
    identifier(T&& value)
      : _any(std::forward<T>(value)),
        _hash(make_hasher(value)),
        _cmp(make_cmp(value)),
        _printer(make_printer(value)) { }

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

    template<typename T>
    identifier &operator=(T&& value)
    {
      *this = identifier(std::forward<T>(value));
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

    std::string to_string() const {
      return _printer(_any);
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

    template<typename T>
    printer_t make_printer(T const&) {
      return [](std::any const& me) {
        T const *v = std::any_cast<T>(&me);
        black_assert(v != nullptr);

        return fmt::format("{}", *v);
      };
    }
  };
}

template<> 
struct fmt::formatter<black::logic::internal::identifier> 
  : fmt::formatter<string_view>
{
  template <typename FormatContext>
  auto 
  format(black::logic::internal::identifier const& p, FormatContext& ctx) const 
  {
    return formatter<string_view>::format(p.to_string(), ctx);
  }
};

template<>
struct std::hash<black::logic::internal::identifier> {
  size_t operator()(black::logic::internal::identifier const&h) const {
    return h.hash();
  }
};

namespace black::logic {
  using internal::identifier;
}

#endif
