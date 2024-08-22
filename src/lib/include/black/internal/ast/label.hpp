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
  concept formattable = std::is_constructible_v<std::formatter<T>>;

  template<typename T>
  concept identifiable = 
    std::equality_comparable<T> && support::hashable<T> && formattable<T>;

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
    explicit label(T value) : _any(wrap(std::move(value))) { }

    template<size_t N>
    label(const char (&str)[N])
      : label(std::string{str}) { }

    label &operator=(label const&) = default;
    label &operator=(label&&) = default;

    bool operator==(label const&) const = default;

    template<typename T>
    label &operator=(T&& value)
    {
      *this = label(std::forward<T>(value));
      return *this;
    }

    size_t hash() const {
      return _any.has_value() ? _any->hash() : 0;
    }

    std::string to_string() const {
      using namespace std::literals;
      return _any.has_value() ? _any->to_string() : "<empty label>"s;
    }

    template<typename T>
    bool is() const {
      return extract<T>(&_any) != nullptr;
    }

    template<typename T>
    std::optional<T> to() const & {
      if(T const*ptr = extract<T>(&_any); ptr)
        return std::optional<T>{*ptr};
      return std::nullopt;
    }

    template<typename T>
    std::optional<T> to() && {
      if(T const*ptr = extract<T>(&_any); ptr)
        return std::optional<T>{std::move(*ptr)};
      return std::nullopt;
    }

    template<typename T>
    T const* get() const { return extract<T>(&_any); }

    template<typename T>
    T *get() { return extract<T>(&_any); }

  private:
    struct label_t {
      label_t() = default;
      label_t(label_t const&) = default;
      label_t &operator=(label_t const&) = default;

      bool operator==(label_t const&) const = default;

      virtual ~label_t() = default;
      virtual size_t hash() const = 0;
      virtual std::string to_string() const = 0;
    };

    template<typename T>
    support::any<label_t> wrap(T arg) {
      struct wrap_t : label_t {
        T _v;

        wrap_t() = default;
        wrap_t(T v) : _v{std::move(v)} { }

        bool operator==(wrap_t const&) const = default;

        virtual size_t hash() const override { return support::hash(_v); }
        virtual std::string to_string() const override { 
          return std::format("{}", _v); 
        }
      }; 
      
      return support::any<label_t>{wrap_t{std::move(arg)}};
    }

    support::any<label_t> wrap(std::string_view view) {
      return wrap(std::string(view));
    }

    support::any<label_t> wrap(const char *str) {
      return wrap(std::string(str));
    }

    support::any<label_t> _any;
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
