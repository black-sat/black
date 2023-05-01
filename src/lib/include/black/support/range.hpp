//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#ifndef BLACK_SUPPORT_RANGE_HPP
#define BLACK_SUPPORT_RANGE_HPP

#include <black/support/assert.hpp>
#include <black/support/utils.hpp>

#include <ranges>
#include <cstdint>

namespace black::support::internal 
{
  class range_iterator;

  //
  // range type providing an integer sequence [begin, end], extrema included.
  //
  // The range is empty if `end < begin`.
  //
  class range 
  {
  public:
    using value_type = int64_t;
    using reference = int64_t const&;
    using const_reference = size_t const&;
    using iterator = range_iterator;
    using const_iterator = range_iterator;

    range(int64_t begin, int64_t end) : _begin{begin}, _end{end} { }

    range_iterator begin() const;
    std::default_sentinel_t end() const { return std::default_sentinel; }

  private:
    int64_t _begin;
    int64_t _end;
  };

  class range_iterator 
  {
  public:
    using value_type = int64_t;
    using difference_type = int64_t;
    using reference = int64_t const&;
    using const_reference = int64_t const&;

    range_iterator() = default;
    range_iterator(int64_t value, int64_t end) 
      : _value{value}, _end{end} { }

    // ForwardIterator requirements
    reference operator*() const {
      return _value;
    }

    range_iterator &operator++() {
      ++_value;
      return *this;
    }

    range_iterator operator++(int) {
      range_iterator tmp{*this};
      ++_value;
      return tmp;
    }

    bool operator==(range_iterator const&other) const = default;

    friend bool operator==(range_iterator it, std::default_sentinel_t) {
      return it._value > it._end;
    }
    
    friend bool operator==(std::default_sentinel_t, range_iterator it) {
      return it._value > it._end;
    }
    
  private:
    int64_t _value = 0;
    int64_t _end = 0;
  };

  inline auto begin(range r) { return r.begin(); } 
  inline auto end(range r) { return r.end(); } 

  inline range_iterator range::begin() const { 
    return range_iterator{_begin, _end}; 
  }

  static_assert(std::input_or_output_iterator<range::iterator>);
  static_assert(std::ranges::range<range>);
} // namespace black::support::internal

namespace black::support {
  using internal::range;
}

#endif // BLACK_SUPPORT_RANGE_HPP
