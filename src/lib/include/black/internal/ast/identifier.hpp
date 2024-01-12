//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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

#ifndef BLACK_LOGIC_IDENTIFIER_HPP
#define BLACK_LOGIC_IDENTIFIER_HPP

#include <black/support>

#include <ranges>

namespace black::ast::core::internal {

  struct root_id_t { };

  class identifier 
  {
  public:
    template<typename T, typename ...Tags>
      requires (
        std::is_constructible_v<label, T> &&
        (std::is_constructible_v<label, Tags> && ...)
      )
    constexpr identifier(T&& label, Tags&& ...tags) 
      : _label{std::forward<T>(label)}, _tags{std::forward<Tags>(tags)...} { }

    template<size_t N>
    identifier(const char (&str)[N])
      : _label{str} { }

    template<std::ranges::range R>
      requires std::is_constructible_v<label, std::ranges::range_value_t<R>>
    identifier(label l, R const& tags) 
      : _label{l}, _tags(begin(tags), end(tags)) { }

    identifier(identifier const&) = default;
    identifier(identifier &&) = default;
    identifier &operator=(identifier const&) = default;
    identifier &operator=(identifier &&) = default;

    bool operator==(identifier const&) const = default;

    class label const& label() const { return _label; }
    std::vector<class label> const& tags() const { return _tags; }

    size_t hash() const {
      return support::hash(label(), tags());
    }

  private:
    class label _label;
    std::vector<class label> _tags;
  };


  class path 
  {
  public:
    path(identifier id) : _ids{id} { }

    template<size_t N>
    path(const char (&str)[N]) : _ids{identifier{str}} { }

    template<std::ranges::range R>
      requires 
        std::is_constructible_v<identifier, std::ranges::range_value_t<R>>
    path(R const& r) : _ids(begin(r), end(r)) { }

    path(path const&) = default;
    path(path &&) = default;
    path &operator=(path const&) = default;
    path &operator=(path &&) = default;

    bool operator==(path const&) const = default;

    std::vector<identifier> const& components() const {
      return _ids;
    }

    size_t hash() const {
      return support::hash(components());
    }

  private:
    std::vector<identifier> _ids;
  };

  inline path operator/(path const &path1, path const &path2) {
    std::vector<identifier> ids = path1.components();
    ids.insert(end(ids), begin(path2.components()), end(path2.components()));

    return path{ids};
  }

}

namespace black::ast::core {
  using internal::identifier;
  using internal::path;
}

#endif // BLACK_LOGIC_IDENTIFIER_HPP
