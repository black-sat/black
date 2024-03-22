//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_PRIVATE_IMMUTABLE_HPP
#define BLACK_PRIVATE_IMMUTABLE_HPP

#include <immer/vector.hpp>
#include <immer/map.hpp>
#include <immer/set.hpp>
#include <immer/algorithm.hpp>


namespace black::support::immutable {

  
  template<typename T>
  class vector 
  {
  public:
    using immer_t = ::immer::vector<T>;
    using value_type = typename immer_t::value_type;
    using reference = typename immer_t::reference;
    using size_type = typename immer_t::size_type;
    using difference_type = typename immer_t::difference_type;
    using const_reference = typename immer_t::const_reference;
    using iterator = typename immer_t::iterator;
    using const_iterator = typename immer_t::const_iterator;
    using reverse_iterator = typename immer_t::reverse_iterator;

    vector() = default;

    vector(std::initializer_list<T> data) : _data{data} { }

    // template<typename Iter, typename Send>
    //   requires std::is_constructible_v<immer_t, Iter, Send>
    // vector(Iter begin, Send end) : _data(begin, end) { }

    // vector(size_type n, T v = {}) : _data(n, v) { }

    vector(vector const&) = default;
    vector(vector &&) = default;

    vector &operator=(vector const&) = default;
    vector &operator=(vector &&) = default;

    bool operator==(vector const&) const = default;

    //
    // All relays of immer::vector<T> functions
    //
    iterator begin() const { return _data.begin(); }
    iterator end() const { return _data.end(); }
    reverse_iterator rbegin() const { return _data.rbegin(); }
    reverse_iterator rend() const { return _data.rend(); }
    size_type size() const { return _data.size(); }
    bool empty() const { return _data.empty(); }
    T const& back() const { return _data.back(); }
    T const& front() const { return _data.front(); }
    reference operator[](size_type index) const { return _data[index]; }
    reference at(size_type index) const { return _data.at(index); }

    //
    // Functions where this differs from immer::vector<T>
    //
    void push_back(value_type value) {
      _data = _data.push_back(value);
    }

    void set(size_type index, value_type value) {
      _data = _data.set(index, value);
    }

    template<typename F>
    void update(size_type index, F f) {
      _data = _data.update(index, f);
    }

    void take(size_type elems) {
      _data = _data.take(elems);
    }

    immer_t immer() const { return _data; }

  private:
    immer_t _data;
  };


}


#endif // BLACK_PRIVATE_IMMUTABLE_HPP
