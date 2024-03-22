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

#ifndef BLACK_PRIVATE_PERSISTENT_HPP
#define BLACK_PRIVATE_PERSISTENT_HPP

#include <immer/vector.hpp>
#include <immer/map.hpp>
#include <immer/set.hpp>
#include <immer/algorithm.hpp>


namespace black::support::persistent {

  
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

    template<typename Iter, typename Send>
      requires std::is_constructible_v<immer_t, Iter, Send>
    vector(Iter begin, Send end) : _data(begin, end) { }

    vector(size_type n, T v = {}) : _data(n, v) { }

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


  template<typename K, typename T>
  class map 
  {
  public:
    using immer_t = ::immer::map<K, T>;
    using key_type = typename immer_t::key_type;
    using mapped_type = typename immer_t::mapped_type;
    using value_type = typename immer_t::value_type;
    using size_type = typename immer_t::size_type;
    using difference_type = typename immer_t::diference_type;
    using hasher = typename immer_t::hasher;
    using key_equal = typename immer_t::key_equal;
    using reference = typename immer_t::reference;
    using const_reference = typename immer_t::const_reference;
    using iterator = typename immer_t::iterator;
    using const_iterator = typename immer_t::const_iterator;

    map() = default;

    map(std::initializer_list<value_type> data) : _data{data} { }

    template<typename Iter, typename Send>
      requires std::is_constructible_v<immer_t, Iter, Send>
    map(Iter begin, Send end) : _data(begin, end) { }

    map(size_type n, T v = {}) : _data(n, v) { }

    map(map const&) = default;
    map(map &&) = default;

    map &operator=(map const&) = default;
    map &operator=(map &&) = default;

    bool operator==(map const&) const = default;

    //
    // All relays of immer::map<K, T> functions
    //
    iterator begin() const { return _data.begin(); }
    iterator end() const { return _data.end(); }
    size_type size() const { return _data.size(); }
    bool empty() const { return _data.empty(); }
    size_type count(K const& k) const { return _data.count(k); }
    T const& operator[](K const& k) const { return _data[k]; }
    T const& at(K const& k) const { return _data.at(k); }
    T const *find(K const& k) const { return _data.find(k); }

    //
    // Functions where this differs from immer::map<K, T>
    //
    void insert(value_type pair) {
      _data = _data.insert(pair);
    }

    void set(key_type key, mapped_type value) {
      _data = _data.set(key, value);
    }

    template<typename F>
    void update(key_type key, F f) {
      _data = _data.update(key, f);
    }

    template<typename F>
    void update_if_exists(key_type key, F f) {
      _data = _data.update_if_exists(key, f);
    }

    void erase(K const& key) {
      _data = _data.erase(key);
    }

    immer_t immer() const { return _data; }

  private:
    immer_t _data;
  };


  template<typename T>
  class set 
  {
  public:
    using immer_t = ::immer::set<T>;
    using value_type = typename immer_t::value_type;
    using size_type = typename immer_t::size_type;
    using difference_type = typename immer_t::diference_type;
    using hasher = typename immer_t::hasher;
    using key_equal = typename immer_t::key_equal;
    using reference = typename immer_t::reference;
    using const_reference = typename immer_t::const_reference;
    using iterator = typename immer_t::iterator;
    using const_iterator = typename immer_t::const_iterator;

    set() = default;

    set(std::initializer_list<value_type> data) : _data{data} { }

    template<typename Iter, typename Send>
      requires std::is_constructible_v<immer_t, Iter, Send>
    set(Iter begin, Send end) : _data(begin, end) { }

    set(size_type n, T v = {}) : _data(n, v) { }

    set(set const&) = default;
    set(set &&) = default;

    set &operator=(set const&) = default;
    set &operator=(set &&) = default;

    bool operator==(set const&) const = default;

    //
    // All relays of immer::set<T> functions
    //
    iterator begin() const { return _data.begin(); }
    iterator end() const { return _data.end(); }
    size_type size() const { return _data.size(); }
    bool empty() const { return _data.empty(); }
    size_type count(T const& v) const { return _data.count(v); }
    T const *find(value_type const& k) const { return _data.find(k); }

    //
    // Functions where this differs from immer::set<T>
    //
    void insert(value_type value) {
      _data = _data.insert(value);
    }

    void erase(value_type const &value) {
      _data = _data.erase(value);
    }

    immer_t immer() const { return _data; }

  private:
    immer_t _data;
  };

}


#endif // BLACK_PRIVATE_PERSISTENT_HPP
