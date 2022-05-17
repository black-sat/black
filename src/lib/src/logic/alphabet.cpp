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

#include <black/logic/alphabet.hpp>

#include <deque>
#include <tsl/hopscotch_map.h>

namespace std {
  template<>
  struct hash<std::vector<::black::internal::term_base *>>
  {
    size_t operator()(
      std::vector<::black::internal::term_base *> const&v
    ) const {
      using namespace ::black::internal;

      hash<term_base *> h;
      size_t result = 0;
      for(size_t i = 0; i < v.size(); ++i)
        result = hash_combine(result, h(v[i]));

      return result;
    }
  };
}
  

namespace black::internal {
  
  struct alphabet::alphabet_impl
  {
    alphabet_impl(alphabet *sigma) : _sigma{sigma} {}

    alphabet            *_sigma;

    std::optional<sort> _domain{};

    boolean_t            _top{true};
    boolean_t            _bottom{false};

    std::deque<proposition_t> _props;
    std::deque<atom_t>        _atoms;
    std::deque<quantifier_t>  _quantifiers;
    std::deque<unary_t>       _unaries;
    std::deque<binary_t>      _binaries;

    std::deque<constant_t>    _consts;
    std::deque<variable_t>    _vars;
    std::deque<next_t>        _nexts;
    std::deque<wnext_t>       _wnexts;
    std::deque<application_t> _apps;

    using unary_key = std::tuple<unary::type, formula_base*>;
    using binary_key = std::tuple<binary::type,
                                  formula_base*,
                                  formula_base*>;
    using quantifier_key = std::tuple<
      quantifier_type, variable_t *, formula_base *
    >;
    using application_key = std::tuple<function, std::vector<term_base *>>;
    using atom_key = std::tuple<relation, std::vector<term_base *>>;
    using constant_key = std::variant<int64_t, double>;

    tsl::hopscotch_map<identifier,     proposition_t*> _props_map;
    tsl::hopscotch_map<atom_key,       atom_t*>        _atoms_map;
    tsl::hopscotch_map<quantifier_key, quantifier_t*>  _quantifiers_map;
    tsl::hopscotch_map<unary_key,      unary_t*>       _unaries_map;
    tsl::hopscotch_map<binary_key,     binary_t*>      _binaries_map;

    tsl::hopscotch_map<constant_key,    constant_t*>    _consts_map;
    tsl::hopscotch_map<term_base *,     next_t*>        _nexts_map;
    tsl::hopscotch_map<term_base *,     wnext_t*>       _wnexts_map;
    tsl::hopscotch_map<application_key, application_t*> _apps_map;
  };

  alphabet::alphabet()
    : _impl{std::make_unique<alphabet_impl>(this)} {}

  alphabet::~alphabet() = default;

  alphabet::alphabet(alphabet&&) = default;
  alphabet &alphabet::operator=(alphabet&&) = default;

  void alphabet::set_domain(sort s) {
    _impl->_domain = s;
  }

  std::optional<sort> alphabet::domain() const {
    return _impl->_domain;
  }

  boolean alphabet::boolean(bool value) {
    return value ? top() : bottom();
  }

  boolean alphabet::top() {
    return internal::boolean{this, &_impl->_top};
  }

  boolean alphabet::bottom() {
    return internal::boolean{this, &_impl->_bottom};
  }

  formula alphabet::from_id(formula_id id) {
    return
    formula{this, reinterpret_cast<formula_base *>(static_cast<uintptr_t>(id))};
  }

  term alphabet::from_id(term_id id) {
    return
    term{this, reinterpret_cast<term_base *>(static_cast<uintptr_t>(id))};
  }

  proposition_t *alphabet::allocate_proposition(identifier _label)
  {
    identifier label{FWD(_label)};

    if(auto it = _impl->_props_map.find(label); it != _impl->_props_map.end())
      return it->second;

    proposition_t *p = &_impl->_props.emplace_back(label);
    _impl->_props_map.insert({label, p});

    return p;
  }

  atom_t *alphabet::allocate_atom(
    relation const&r, std::vector<term_base *> const&terms
  ) {
    if(auto it = 
        _impl->_atoms_map.find({r, terms}); it != _impl->_atoms_map.end())
      return it->second;

    atom_t *a = &_impl->_atoms.emplace_back(r, terms);
    _impl->_atoms_map.insert({{r, terms}, a});

    return a;
  }

  quantifier_t *alphabet::allocate_quantifier(
    quantifier_type type, variable_t *var, formula_base *matrix
  ) {
    if(auto it = _impl->_quantifiers_map.find({type, var, matrix}); 
            it != _impl->_quantifiers_map.end())
      return it->second;

    quantifier_t *q = &_impl->_quantifiers.emplace_back(type, var, matrix);
    _impl->_quantifiers_map.insert({{type, var, matrix}, q});

    return q;
  }

  variable_t *alphabet::allocate_variable(identifier _label)
  {
    identifier label{FWD(_label)};
    return &_impl->_vars.emplace_back(label);
  }

  unary_t *alphabet::allocate_unary(unary::type type, formula_base* arg)
  {
    auto it = _impl->_unaries_map.find({type, arg});
    if(it != _impl->_unaries_map.end())
      return it->second;

    unary_t *f =
      &_impl->_unaries.emplace_back(static_cast<formula_type>(type), arg);
    _impl->_unaries_map.insert({{type, arg}, f});

    return f;
  }

  binary_t *alphabet::allocate_binary(
    binary::type type, formula_base* arg1, formula_base* arg2
  ) {
    auto it = _impl->_binaries_map.find({type, arg1, arg2});
    if(it != _impl->_binaries_map.end())
      return it->second;

    binary_t *f =
      &_impl->_binaries.emplace_back(
        static_cast<formula_type>(type), arg1, arg2 // LCOV_EXCL_LINE
      );
    _impl->_binaries_map.insert({{type, arg1, arg2}, f});

    return f;
  }

  constant_t *alphabet::allocate_constant(int64_t c)
  {
    auto it = _impl->_consts_map.find(c);
    if(it != _impl->_consts_map.end())
      return it->second;

    constant_t *t = &_impl->_consts.emplace_back(c);
    _impl->_consts_map.insert({c,t});

    return t;
  }

  constant_t *alphabet::allocate_constant(double c)
  {
    auto it = _impl->_consts_map.find(c);
    if(it != _impl->_consts_map.end())
      return it->second;

    constant_t *t = &_impl->_consts.emplace_back(c);
    _impl->_consts_map.insert({c,t});

    return t;
  }

  next_t *alphabet::allocate_next(term_base *arg)
  {
    auto it = _impl->_nexts_map.find(arg);
    if(it != _impl->_nexts_map.end())
      return it->second;

    next_t *t = &_impl->_nexts.emplace_back(arg);
    _impl->_nexts_map.insert({arg, t});

    return t;
  }

  wnext_t *alphabet::allocate_wnext(term_base *arg)
  {
    auto it = _impl->_wnexts_map.find(arg);
    if(it != _impl->_wnexts_map.end())
      return it->second;

    wnext_t *t = &_impl->_wnexts.emplace_back(arg);
    _impl->_wnexts_map.insert({arg, t});

    return t;
  }

  application_t *alphabet::allocate_application(
    function const& func, std::vector<term_base *> const&args
  )
  {
    auto it = _impl->_apps_map.find({func, args});
    if(it != _impl->_apps_map.end())
      return it->second;

    application_t *t = &_impl->_apps.emplace_back(func, args);
    _impl->_apps_map.insert({{func, args}, t});

    return t; 
  }

}
