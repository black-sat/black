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
//

#ifndef BLACK_LOGIC_RENAMINGS_HPP
#define BLACK_LOGIC_RENAMINGS_HPP

#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/support/identifier.hpp>

#include <unordered_set>
#include <string>
#include <ranges>
#include <iostream>

namespace black_internal::renamings {

  using rename_cache_t = std::unordered_map<
    logic::formula<logic::propositional>,
    logic::formula<logic::propositional>
  >;

  inline logic::formula<logic::propositional> 
  rename(
    logic::formula<logic::propositional> f, 
    std::function<logic::proposition(logic::proposition)> map,
    rename_cache_t &cache
  ) {
    using namespace black::logic::fragments::propositional;
    if(cache.contains(f))
      return cache.at(f);

    auto result = f.match(
      [](boolean b) { return b; },
      [&](proposition p) {
        return map(p);
      },
      [&](unary u, auto arg) {
        return unary(u.node_type(), rename(arg, map, cache));
      },
      [&](conjunction c) {
        return big_and(*c.sigma(), c.operands(), [&](auto op) {
          return rename(op, map, cache);
        });
      },
      [&](disjunction d) {
        return big_or(*d.sigma(), d.operands(), [&](auto op) {
          return rename(op, map, cache);
        });
      },
      [&](binary b, auto left, auto right) {
        return binary(
          b.node_type(), rename(left, map, cache), rename(right, map, cache)
        );
      }
    );

    cache.insert({f, result});
    return result;
  }

  inline logic::formula<logic::propositional> 
  rename(
    logic::formula<logic::propositional> f, 
    std::function<logic::proposition(logic::proposition)> map
  ) {
    rename_cache_t cache;
    return rename(f, map, cache);
  }

  inline std::vector<logic::proposition>
  rename(
    std::vector<logic::proposition> const&props, 
    std::function<logic::proposition(logic::proposition)> map
  ) {
    std::vector<logic::proposition> result;
    for(auto p : props)
      result.push_back(map(p));
    return result;
  }
  
  struct tag_t {
    black::proposition base;
    size_t primes = 0;
    int64_t steps = -1;

    bool operator==(tag_t const&t) const = default;
  };

  inline std::string to_string(tag_t t) {
    using namespace std::literals;
    return to_string(t.base) + 
          (t.steps >= 0 ? "@" + std::to_string(t.steps) : ""s) +
          (t.primes > 0 ? ":" + std::to_string(t.primes) : ""s);
  }

}

template<>
struct std::hash<black_internal::renamings::tag_t> {
  size_t operator()(black_internal::renamings::tag_t t) const {
    return 
      black_internal::hash_combine(
        std::hash<black::proposition>{}(t.base),
        std::hash<int64_t>{}(int64_t(t.primes) + t.steps)
      );
  }
};

namespace black_internal::renamings {

  inline bool is_plain(black::proposition p) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      return true;
    return false;
  }

  inline bool is_primed(black::proposition p, size_t n) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      if(tag->primes == n)
        return true;
    return false;
  }

  inline bool is_primed(black::proposition p) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      if(tag->primes > 0)
        return true;
    return false;
  }

  inline bool is_stepped(black::proposition p, size_t n) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      if(tag->steps == int64_t(n))
        return true;
    return false;
  }
  
  inline bool is_stepped(black::proposition p) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      if(tag->steps >= 0)
        return true;
    return false;
  }

  inline black::proposition untag(black::proposition p) {
    tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
    return tag.base;
  }

  inline black::proposition prime(black::proposition p, size_t n) {
    tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
    tag.primes = n;
    return p.sigma()->proposition(tag);
  }

  inline black::proposition step(black::proposition p, size_t n) {
    tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
    tag.steps = int64_t(n);
    return p.sigma()->proposition(tag);
  }
  
  inline black::proposition prime(black::proposition p) {
    tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
    tag.primes++;
    return p.sigma()->proposition(tag);
  }

  inline black::proposition step(black::proposition p) {
    tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
    tag.steps++;
    return p.sigma()->proposition(tag);
  }

  //
  // f[stepped(n) * vars / plain()]
  // f[stepped(n) * vars / stepped(n + 1)]
  //
  // in the above, the first replaces any variable that is the n-th stepped
  // version of a variable from the vector `vars`, with its plain version and
  // its n+1 stepped version, respectively
  //

  template<typename R>
  concept renamer = requires(R r, black::proposition p) {
    { r.rename(p) } -> std::convertible_to<black::proposition>;
  };

  template<typename M>
  concept filter = requires(M m, black::proposition p) {
    { m.filter(p) } -> std::convertible_to<bool>;
  };

  template<filter M, renamer R>
  auto operator/(M const& m, R const &r) {
    return [=](black::proposition p) {
      if(m.filter(p))
        return r.rename(p);
      return p;
    };
  }

  template<filter M1, filter M2>
  auto operator *(M1 const& m1, M2 const& m2) {
    struct result_t {

      result_t(M1 const& _m1_, M2 const& _m2_) : _m1{_m1_}, _m2{_m2_} { }

      bool filter(black::proposition p) const {
        return _m1.filter(p) && _m2.filter(p);
      }

      M1 _m1;
      M2 _m2;

    } result{m1, m2};

    return result;
  }

  template<filter M1, filter M2>
  auto operator+(M1 const& m1, M2 const& m2) {
    struct result_t {

      result_t(M1 const& _m1_, M2 const& _m2_) : _m1{_m1_}, _m2{_m2_} { }

      bool filter(black::proposition p) const {
        return _m1.filter(p) || _m2.filter(p);
      }

      M1 _m1;
      M2 _m2;

    } result{m1, m2};

    return result;
  }

  template<filter M>
  auto operator!(M const& m) {
    struct result_t {

      result_t(M const& _m_) : _m{_m_} { }

      bool filter(black::proposition p) const {
        return !_m.filter(p);
      }

      M _m;

    } result{m};

    return result;
  }

  struct of_kind {
    
    of_kind(std::vector<black::proposition> const& vec) {
      for(auto p : vec)
        set.insert(p);
    }

    bool filter(black::proposition p) const {
      return set.contains(untag(p));
    }

    std::unordered_set<black::proposition> set;
  };
  
  struct exactly {
    
    exactly(std::vector<black::proposition> const&vec) {
      for(auto p : vec)
        set.insert(p);
    }

    bool filter(black::proposition p) const {
      return set.contains(p);
    }

    std::unordered_set<black::proposition> set;
  };

  template<std::ranges::range T, renamer R>
  auto operator/(T&& v, R&& r) {
    return exactly(std::forward<T>(v)) / std::forward<R>(r);
  }

  template<filter M1, std::ranges::range T2>
  auto operator *(M1 const& m1, T2 const& v2) {
    return m1 * of_kind(v2);
  }

  template<filter M2, std::ranges::range T1>
  auto operator *(T1 const& v1, M2 const& m2) {
    return of_kind(v1) * m2;
  }

  struct plain {

    plain() = default;

    bool filter(black::proposition p) const {
      return is_plain(p);
    }

    black::proposition rename(black::proposition p) const {
      return untag(p);
    }

  };

  struct primed {

    primed() { }
    primed(size_t _n) : n{_n} { }

    bool filter(black::proposition p) const {
      if(n)
        return is_primed(p, *n);
      return is_primed(p);
    }

    black::proposition rename(black::proposition p) const {
      if(n)
        return prime(p, *n);
      return prime(p);
    }

    std::optional<size_t> n;
  };

  struct stepped {

    stepped() { }
    stepped(size_t _n) : n{_n} { }

    bool filter(black::proposition p) const {
      if(n)
        return is_stepped(p, *n);
      return is_stepped(p);
    }

    black::proposition rename(black::proposition p) const {
      if(n) {
        if(!add)
          return step(p, *n);
        
        tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
        tag.steps += *n;

        return p.sigma()->proposition(tag);
      }
      return step(p);
    }

    stepped operator+() const {
      stepped copy = *this;
      copy.add = true;
      return copy;
    }

    std::optional<size_t> n;
    bool add = false;
  };

  struct only {

    template<renamer R>
    only(R const& r) 
      : base{[=](black::proposition p) {
        return r.rename(p);
      }} { }

    black::proposition rename(black::proposition p) const {
      return base(untag(p));
    }

    std::function<black::proposition(black::proposition)> base;
  };

}

namespace black_internal {
  using namespace renamings;
}

#endif // BLACK_LOGIC_RENAMINGS_HPP
