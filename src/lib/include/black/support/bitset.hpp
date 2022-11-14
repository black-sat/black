//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_SUPPORT_BITSET_HPP
#define BLACK_SUPPORT_BITSET_HPP

#include <cstdint>
#include <cstddef>
#include <limits>
#include <concepts>

namespace black_internal::bitset {
  
  //
  // This is a simple replacement of the `std::bitset` class with the difference
  // that all member and non-member functions are `constexpr` (this will be the
  // case for `std::bitset` only from C++23).
  //
  template<size_t N>
  class bitset 
  {
    static_assert(N > 0);

  public:

    constexpr bitset() = default;
    constexpr bitset(bitset const&) = default;

    constexpr bitset(std::initializer_list<size_t> args) {
      for(size_t i : args)
        set(i);
    }

    constexpr bitset &operator=(bitset const&) = default;

    constexpr bool contains(size_t i) const {
      if(i >= N)
        return false;

      size_t word = i / _word;
      size_t index = i % _word;

      return (_data[word] >> index) & 1;
    }

    constexpr void set(size_t i) {
      if(i >= N)
        return;

      size_t word = i / _word;
      size_t index = i % _word;

      _data[word] = _data[word] | (uint64_t{1} << index);
    }

    constexpr void reset(size_t i) {
      if(i >= N)
        return;

      size_t word = i / _word;
      size_t index = i % _word;

      _data[word] = _data[word] & ~(uint64_t{1} << index);
    }

    friend constexpr bool operator==(bitset const& b1, bitset const& b2) {
      for(size_t i = 0; i < _width - 1; ++i)
        if(b1._data[i] != b2._data[i])
          return false;

      uint64_t rem = N % _word;
      uint64_t mask = ~(std::numeric_limits<uint64_t>::max() << rem);
      return (b1._data[_width - 1] & mask) == (b2._data[_width - 1] & mask);
    }

    friend constexpr bitset operator&(bitset const& b1, bitset const& b2) {
      bitset result;

      for(size_t i = 0; i < _width; ++i)
        result._data[i] = b1._data[i] & b2._data[i];
      
      return result;
    }

    friend constexpr bitset operator|(bitset const& b1, bitset const& b2) {
      bitset result;

      for(size_t i = 0; i < _width; ++i)
        result._data[i] = b1._data[i] | b2._data[i];
      
      return result;
    }

    friend constexpr bitset operator~(bitset b) {
      for(size_t i = 0; i < _width; ++i) {
        b._data[i] = ~b._data[i];
      }
      return b;
    }

    //
    // Debug function to print bitsets
    //
    friend std::string to_string(bitset f) {
      std::string s = "";
      for(size_t i = 0; i < N; ++i) {
        if(i != 0 && i % 4 == 0)
          s += " ";
        s += f.contains(i) ? "1" : "0";
      }
      return s;
    }

    constexpr bool contains(bitset const& other) const {
      for(size_t i = 0; i < N; ++i)
        if(other.contains(i) && !contains(i))
          return false;
      return true;
      //return *this == (*this | other);
    }

  private:
    static constexpr size_t _word = sizeof(uint64_t) * 8;
    static constexpr size_t _width = 
      (N / _word) + ((N % _word) ? 1 : 0);
    
    uint64_t _data[_width] = {};
  };

}

namespace black {
  using black_internal::bitset::bitset;
}

#endif // BLACK_SUPPORT_BITSET_HPP
