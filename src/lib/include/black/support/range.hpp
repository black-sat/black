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

#ifndef BLACK_RANGE_HPP
#define BLACK_RANGE_HPP

#include <black/support/assert.hpp>

namespace black::internal 
{
  class range_iterator;
  class range_end_iterator;

  class range 
  {
  public:
    using value_type = size_t;
    using reference = size_t const&;
    using const_reference = size_t const&;
    using iterator = range_iterator;
    using const_iterator = range_iterator;

    range(size_t begin, size_t end) : _begin{begin}, _end{end} { }

    range_iterator begin() const;
    range_end_iterator end() const;

  private:
    size_t _begin;
    size_t _end;
  };

  class range_iterator {
  public:
    friend class range_end_iterator;

    using value_type = size_t;
    using reference = size_t const&;
    using const_reference = size_t const&;

    range_iterator(size_t value) : _value{value} { }

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

    bool operator==(range_iterator other) const {
      return _value == other._value;
    }

    bool operator!=(range_iterator other) const {
      return _value != other._value;
    }

    bool operator==(range_end_iterator end);
    bool operator!=(range_end_iterator end);

  private:
    size_t _value;
  };

  class range_end_iterator 
  {
    friend class range_iterator;
  public:
    range_end_iterator(size_t end) : _end{end} { }

    bool operator==(range_iterator it);
    bool operator!=(range_iterator it);

  private:
    size_t _end;
  };

  inline bool range_iterator::operator==(range_end_iterator end) {
    return _value == end._end;
  }

  inline bool range_iterator::operator!=(range_end_iterator end) {
    return _value != end._end;
  }

  inline bool range_end_iterator::operator==(range_iterator it) {
    return _end == it._value;
  }

  inline bool range_end_iterator::operator!=(range_iterator it) {
    return _end != it._value;
  }

  inline range_iterator begin(range r) { return r.begin(); } 
  inline range_end_iterator end(range r) { return r.end(); } 

  inline range_iterator range::begin() const { 
    return range_iterator{_begin}; 
  }

  inline range_end_iterator range::end() const { 
    return range_end_iterator{_end};
  }
} // namespace black::internal

namespace black {
  using internal::range;
};

#endif // BLACK_RANGE_HPP
