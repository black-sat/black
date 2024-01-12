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

#ifndef BLACK_SUPPORT_LABEL_HPP
#define BLACK_SUPPORT_LABEL_HPP

#include <black/support>

#include <format>

#include <any>
#include <tuple>
#include <optional>
#include <vector>
#include <string_view>
#include <string>

namespace black::ast::core::internal
{
  class label;

  template<typename T>
  concept formattable = std::is_constructible<std::formatter<T>>::value;

  template<typename T>
  concept identifiable = 
    !std::is_same_v<T, label> &&
    std::equality_comparable<T> && formattable<T>;

  //
  // Type-erased hashable, comparable and printable value
  //
  class label
  {
  public:
    constexpr label() = default;
    label(label const&) = default;
    label(label&&) = default;

    template<typename T>
      requires identifiable<std::remove_cvref_t<T>>
    label(T&& value)
      : _any(std::forward<T>(value)),
        _hash(make_hasher(value)),
        _cmp(make_cmp(value)),
        _printer(make_printer(value)) { }

    label(std::string_view view) 
      : label{std::string{view}} { }

    label(char const* c_str) 
      : label{std::string{c_str}} { }

    template<size_t N>
    label(const char (&c_str)[N])
      : label{std::string{c_str}} { }

    size_t hash() const {
      return _hash(_any);
    }

    label &operator=(label const&) = default;
    label &operator=(label&&) = default;

    bool operator==(label const&other) const {
      return _cmp(_any, other);
    }

    template<typename T>
    label &operator=(T&& value)
    {
      *this = label(std::forward<T>(value));
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
    using comparator_t = bool (*)(std::any const&, label const&);
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
        if(v == nullptr)
          return 0;

        return support::hash(*v); // LCOV_EXCL_LINE
      };
    }
    
    template<typename T>
    comparator_t make_cmp(T const&) { // LCOV_EXCL_LINE
      return [](std::any const&me, label const&other) -> bool { // LCOV_EXCL_LINE
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

        return std::format("{}", *v);
      };
    }
  };
}

template<> 
struct std::formatter<black::ast::core::internal::label> 
  : std::formatter<string_view>
{
  template <typename FormatContext>
  auto 
  format(black::ast::core::internal::label const& p, FormatContext& ctx) const 
  {
    return formatter<string_view>::format(p.to_string(), ctx);
  }
};

namespace black::ast::core {
  using internal::label;
}

#endif // BLACK_SUPPORT_LABEL_HPP
