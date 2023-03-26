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

#ifndef BLACK_TRIBOOL_HPP
#define BLACK_TRIBOOL_HPP

#include <cstdint>
#include <ostream>

namespace black::support::internal 
{
  class tribool {
    enum tribool_t : uint8_t {
      _false = 0,
      _true = 1,
      _undef = 2,
    };
  
  public:
    struct undef_t { };
    static constexpr undef_t undef = {};

    constexpr tribool() = default;
    constexpr tribool(undef_t) : _value{_undef} { }
    constexpr tribool(bool b) : _value{ b ? _true : _false } { }

    tribool(tribool const&) = default;
    tribool &operator=(tribool const&) = default;

    bool operator==(tribool const &t) const = default;
    
    bool operator==(bool b) const { 
      return b ? _value == _true : _value == _false; 
    }

    friend bool operator==(bool b, tribool t) {
      return b ? t._value == _true : t._value == _false;
    }
    
    bool operator!=(bool b) const { 
      return b ? _value != _true : _value != _false;
    }

    friend bool operator!=(bool b, tribool t) {
      return b ? t._value != _true : t._value != _false;
    }
    
    tribool operator !() const { 
      tribool neg{undef};
      if(_value == _true)
        neg._value = _false;
      if(_value == _false)
        neg._value = _true;
      
      return neg;
    }

    explicit operator bool() const { return _value == _true; }
    
    // GCOV false negatives
    friend std::ostream &
    operator<<(std::ostream &s, tribool b) { // LCOV_EXCL_LINE
      if(b == true) // LCOV_EXCL_LINE
        return s << "true"; // LCOV_EXCL_LINE
      if(b == false) // LCOV_EXCL_LINE
        return s << "false"; // LCOV_EXCL_LINE
      
      return s << "tribool::undef"; // LCOV_EXCL_LINE
    }

  private:
    tribool_t _value = _undef;
  };
}

namespace black::support {
  using internal::tribool;
}

#endif // BLACK_TRIBOOL_HPP
