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

namespace black_internal::renamings {

  struct tag_t {
    black::proposition base;
    size_t primes = 0;
    size_t steps = 0;

    bool operator==(tag_t const&t) const = default;
  };

  inline std::string to_string(tag_t t) {
    return to_string(t.base) + ":" + 
           std::to_string(t.primes) + ":" + std::to_string(t.steps);
  }

}

template<>
struct std::hash<black_internal::renamings::tag_t> {
  size_t operator()(black_internal::renamings::tag_t t) const {
    return 
      black_internal::hash_combine(
        std::hash<black::proposition>{}(t.base),
        std::hash<size_t>{}(t.primes + t.steps)
      );
  }
};

namespace black_internal::renamings {

  inline std::optional<black::proposition>
  is_plain(black::proposition p) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      return {};
    return p;
  }

  inline std::optional<black::proposition>
  is_primed(black::proposition p, size_t n) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      if(tag->primes == n)
        return tag->base;
    return {};
  }

  inline std::optional<black::proposition>
  is_stepped(black::proposition p, size_t n) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      if(tag->steps == n)
        return tag->base;
    return {};
  }

  inline black::proposition make_primed(black::proposition p, size_t n) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      return 
        p.sigma()->proposition(tag_t{tag->base, n, tag->steps});
    return p.sigma()->proposition(tag_t{p, n, 0});
  }

  inline black::proposition make_stepped(black::proposition p, size_t n) {
    if(auto tag = p.name().to<tag_t>(); tag.has_value())
      return 
        p.sigma()->proposition(tag_t{tag->base, tag->primes, n});
    return p.sigma()->proposition(tag_t{p, 0, n});
  }

  //
  // f[stepped(n) * any_of(vars) / plain()]
  // f[stepped(n) * any_of(vars) / stepped(n + 1)]
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
  concept matcher = requires(M m, black::proposition p) {
    { m.match(p) } -> std::convertible_to<std::optional<black::proposition>>;
  };

  template<matcher M, renamer R>
  auto operator/(M&& m, R&& r) {
    return [&](black::proposition p) {
      if(auto base = std::forward<M>(m).match(p); base)
        return std::forward<R>(r).rename(*base);
      return p;
    };
  }

  template<matcher M1, matcher M2>
  auto operator*(M1 const& m1, M2 const& m2) {
    struct result_t {

      result_t(M1 const& _m1_, M2 const& _m2_) : _m1{_m1_}, _m2{_m2_} { }

      std::optional<black::proposition> match(black::proposition p) {
        if(auto base2 = _m2.match(p); base2)
          if(auto base1 = _m1.match(*base2); base1)
            return *base1;
        return {};
      }

      M1 const& _m1;
      M2 const& _m2;

    } result{m1, m2};

    return result;
  }

  template<matcher M1, matcher M2>
  auto operator+(M1 const& m1, M2 const& m2) {
    struct result_t {

      result_t(M1 const& _m1_, M2 const& _m2_) : _m1{_m1_}, _m2{_m2_} { }

      std::optional<black::proposition> match(black::proposition p) {
        if(auto base = _m1.match(p); base)
          return *base;
        if(auto base = _m2.match(p); base)
          return *base;
          
        return {};
      }

      M1 const& _m1;
      M2 const& _m2;

    } result{m1, m2};

    return result;
  }

  struct any_of {
    
    any_of(std::vector<black::proposition> vec) {
      for(auto p : vec)
        set.insert(p);
    }

    std::optional<black::proposition> match(black::proposition p) const {
      if(set.contains(p))
        return p;
      return {};
    }

    std::unordered_set<black::proposition> set;
  };

  template<matcher M>
  auto operator*(M const& m, black::proposition p) {
    return m * any_of({p});
  }

  template<matcher M>
  auto operator+(M const& m, black::proposition p) {
    return m + any_of({p});
  }

  template<matcher M>
  auto operator+(black::proposition p, M const& m) {
    return any_of({p}) + m;
  }

  struct plain {

    plain() = default;

    std::optional<black::proposition> match(black::proposition p) const {
      return is_plain(p);
    }

    black::proposition rename(black::proposition p) const {
      return p;
    }

  };

  struct primed {

    primed(size_t _n) : n{_n} { }

    std::optional<black::proposition> match(black::proposition p) const {
      return is_primed(p, n);
    }

    black::proposition rename(black::proposition p) const {
      return make_primed(p, n);
    }

    size_t n;
  };

  struct stepped {

    stepped(size_t _n) : n{_n} { }

    std::optional<black::proposition> match(black::proposition p) const {
      return is_stepped(p, n);
    }

    black::proposition rename(black::proposition p) const {
      return make_stepped(p, n);
    }

    size_t n;
  };

}

namespace black_internal {
  using namespace renamings;
}

#endif // BLACK_LOGIC_RENAMINGS_HPP
