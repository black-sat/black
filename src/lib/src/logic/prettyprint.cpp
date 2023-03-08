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

#include <black/logic/prettyprint.hpp>
#include <black/logic/lex.hpp>
#include <black/support/to_string.hpp>

#include <tsl/hopscotch_set.h>
#include <tsl/hopscotch_map.h>

#include <fmt/format.h>

namespace black_internal::logic
{

  static
  bool does_need_parens(
    formula<universal_fragment_t> parent, formula<universal_fragment_t> arg
  ) {
    bool parens = false;
    arg.match(
      [&](binary<universal_fragment_t>) {
        parens = 
        (!parent.is<conjunction<universal_fragment_t>>() && 
         !parent.is<disjunction<universal_fragment_t>>()
        ) || (parent.node_type() != arg.node_type());
      },
      [&](comparison<universal_fragment_t>) {
        parens = true;
      },
      [&](quantifier<universal_fragment_t>) {
        parens = true;
      },
      [&](otherwise) { }
    );
    
    return parens;
  }

  static
  std::string to_string(unary<universal_fragment_t>::type t) {
    return t.match(
      [](unary<universal_fragment_t>::type::negation)     { return "!"; },
      [](unary<universal_fragment_t>::type::tomorrow)     { return "X"; },
      [](unary<universal_fragment_t>::type::w_tomorrow)   { return "wX"; },
      [](unary<universal_fragment_t>::type::yesterday)    { return "Y"; },
      [](unary<universal_fragment_t>::type::w_yesterday)  { return "Z"; },
      [](unary<universal_fragment_t>::type::always)       { return "G"; },
      [](unary<universal_fragment_t>::type::eventually)   { return "F"; },
      [](unary<universal_fragment_t>::type::once)         { return "O"; },
      [](unary<universal_fragment_t>::type::historically) { return "H"; }
    );
  }

  static
  std::string to_string(binary<universal_fragment_t>::type t) {
    return t.match(
      [](binary<universal_fragment_t>::type::conjunction) { return "&"; },
      [](binary<universal_fragment_t>::type::disjunction) { return "|"; },
      [](binary<universal_fragment_t>::type::implication) { return "->"; },
      [](binary<universal_fragment_t>::type::iff)         { return "<->"; },
      [](binary<universal_fragment_t>::type::until)       { return "U"; },
      [](binary<universal_fragment_t>::type::release)     { return "R"; },
      [](binary<universal_fragment_t>::type::w_until)     { return "W"; },
      [](binary<universal_fragment_t>::type::s_release)   { return "M"; },
      [](binary<universal_fragment_t>::type::since)       { return "S"; },
      [](binary<universal_fragment_t>::type::triggered)   { return "T"; }
    );
  }

  static
  std::string to_string(equality<universal_fragment_t>::type t, bool binary) {
    if(binary)
      return t.match(
        [](equality<universal_fragment_t>::type::equal)    { return "="; },
        [](equality<universal_fragment_t>::type::distinct) { return "!="; }
      );
    return t.match(
      [](equality<universal_fragment_t>::type::equal)    { return "equal"; },
      [](equality<universal_fragment_t>::type::distinct) { return "distinct"; }
    );
  }

  static
  std::string to_string(comparison<universal_fragment_t>::type t) {
    return t.match(
      [](comparison<universal_fragment_t>::type::less_than)
        { return "<"; },
      [](comparison<universal_fragment_t>::type::less_than_equal)
        { return "<="; },
      [](comparison<universal_fragment_t>::type::greater_than)
        { return ">"; },
      [](comparison<universal_fragment_t>::type::greater_than_equal)
        { return ">="; }
    );
  }

  template<typename T>
  std::string parens_if_needed(T t, bool needs_parens) {
    return needs_parens ? "(" + to_string(t) + ")" : to_string(t);
  }

  static
  std::string escape(std::string s) {
    if(s.empty())
      return "{}";

    if(lexer::is_keyword(s)) {
      return "{" + s + "}";
    }
    
    bool escaped = false;
    if(!lexer::is_initial_identifier_char(s[0]))
      escaped = true;
    size_t i = 0;
    if(s[0] == '}' || s[0] == '\\') {
      s.insert(0, 1, '\\');
      i = 2;
    }

    while(i < s.size()) {
      if(!lexer::is_identifier_char(s[i]))
        escaped = true;
      if(s[i] == '}' || s[i] == '\\') {
        s.insert(i, 1, '\\');
        i++;
      }
      i++;
    }

    if(escaped)
      return "{" + s + "}";

    return s;
  }

  static
  std::string term_parens(term<universal_fragment_t> t) {
    return t.match(
      [&](variable) {
        return to_string(t);
      },
      [&](constant<universal_fragment_t>) {
        return to_string(t);
      },
      [&](otherwise) {
        return "(" + to_string(t) + ")";
      }
    );
  }

  std::string to_string(term<universal_fragment_t> t)
  {
    using namespace std::literals;
    using namespace black_internal;

    return t.match(
      [&](constant<universal_fragment_t> c) {
        return to_string(c.value());
      },
      [&](variable x) {
        return escape(to_string(x.name()));
      },
      [&](application<universal_fragment_t> a) {
        std::string result = 
          escape(to_string(a.func().name())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [&](negative<universal_fragment_t>, auto arg) {
        return fmt::format("-({})", to_string(arg));
      },
      [&](to_integer<universal_fragment_t>, auto arg) {
        return fmt::format("to_int({})", to_string(arg));
      },
      [&](to_real<universal_fragment_t>, auto arg) {
        return fmt::format("to_real({})", to_string(arg));
      },
      [&](next<universal_fragment_t>, auto arg) {
        return fmt::format("next({})", to_string(arg));
      },
      [&](wnext<universal_fragment_t>, auto arg) {
        return fmt::format("wnext({})", to_string(arg));
      },
      [&](prev<universal_fragment_t>, auto arg) {
        return fmt::format("prev({})", to_string(arg));
      },
      [&](wprev<universal_fragment_t>, auto arg) {
        return fmt::format("wprev({})", to_string(arg));
      },
      [&](addition<universal_fragment_t>, auto left, auto right) {
        return fmt::format("{} + {}", term_parens(left), term_parens(right));
      },
      [&](subtraction<universal_fragment_t>, auto left, auto right) {
        return fmt::format("{} - {}", term_parens(left), term_parens(right));
      },
      [&](multiplication<universal_fragment_t>, auto left, auto right) {
        return fmt::format("{} * {}", term_parens(left), term_parens(right));
      },
      [&](division<universal_fragment_t>, auto left, auto right) {
        return fmt::format("{} / {}", term_parens(left), term_parens(right));
      },
      [&](int_division<universal_fragment_t>, auto left, auto right) {
        return fmt::format("{} div {}", term_parens(left), term_parens(right));
      }
    );
  }

  std::string to_string(formula<universal_fragment_t> f)
  {
    using namespace std::literals;
    using namespace ::black_internal;

    return f.match(
      [&](proposition p) {
        return escape(to_string(p.name()));
      },
      [&](atom<universal_fragment_t> a) {
        std::string result = 
          escape(to_string(a.rel().name())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [](equality<universal_fragment_t> e, auto terms) {
        black_assert(terms.size() > 0);

        if(terms.size() == 2) 
          return fmt::format(
            "{} {} {}", 
            to_string(terms[0]), 
            to_string(e.node_type(), true), 
            to_string(terms[1])
          );
        
        std::string args = to_string(terms[0]);
        for(size_t i = 1; i < terms.size(); ++i) 
          args += ", " + to_string(terms[i]);
        
        return fmt::format("{}({})", to_string(e.node_type(), false), args);
      },
      [](comparison<universal_fragment_t> c, auto left, auto right) {
        return fmt::format(
          "{} {} {}", 
          to_string(left), to_string(c.node_type()), to_string(right)
        );
      },
      [](quantifier<universal_fragment_t> q) {
        std::string qs = 
          q.node_type() == quantifier<universal_fragment_t>::type::exists{} ?
            "exists " : "forall ";

        bool parens = q.matrix().is<binary<universal_fragment_t>>();

        for(var_decl d : q.variables()) {
          qs += 
            '(' + to_string(d.variable()) + " : " + to_string(d.sort()) + ") ";
        }

        return fmt::format("{}. {}", qs, parens_if_needed(q.matrix(), parens));
      }, // LCOV_EXCL_LINE
      [](qbf<universal_fragment_t> q) {
        std::string qs = 
          q.node_type() == quantifier<universal_fragment_t>::type::exists{} ?
            "thereis " : "foreach ";

        bool parens = q.matrix().is<binary<universal_fragment_t>>();

        for(proposition p : q.variables()) {
          qs += to_string(p);
        }

        return fmt::format("{}. {}", qs, parens_if_needed(q.matrix(), parens));
      }, // LCOV_EXCL_LINE
      [](boolean, bool b) {
        return b ? "True" : "False";
      },
      [](negation<universal_fragment_t> n, auto arg) {
        bool needs_parens = does_need_parens(n, arg);
        return fmt::format("{}{}", 
          to_string(unary<universal_fragment_t>::type::negation{}), 
          parens_if_needed(arg, needs_parens)
        );
      },
      [](unary<universal_fragment_t> u, auto arg) {
        bool needs_parens = does_need_parens(u, arg);
        return fmt::format("{}{}{}",
                            to_string(u.node_type()),
                            needs_parens ? "" : " ",
                            parens_if_needed(arg, needs_parens));
      },
      [](binary<universal_fragment_t> b, auto left, auto right) {
        return
          fmt::format("{} {} {}",
                      parens_if_needed(left, does_need_parens(b, left)),
                      to_string(b.node_type()),
                      parens_if_needed(right, does_need_parens(b, right)));
      }
    );
  }

  std::string to_string(symbol<universal_fragment_t> s) {
    return s.match(
      [](relation r) {
        return to_string(r.name());
      },
      [](function f) {
        return to_string(f.name());
      }
    );
  }

  std::string to_string(number<universal_fragment_t> n) {
    return n.match(
      [](integer, int64_t value) {
        return fmt::format("{}", value);
      },
      [](real, double value) {
        std::string s = fmt::format("{}", value);
        if(s.find('.') == std::string::npos)
          s += ".0";
        return s;
      }
    );
  }

  std::string to_string(declaration s) {
    return s.match(
      [](var_decl, auto var, auto sort) { 
        return to_string(var) + " : " + to_string(sort); 
      },
      [](rel_decl, auto rel, auto signature) {
        auto res = to_string(rel) + "(" + to_string(signature[0]);
        for(size_t i = 1; i < signature.size(); ++i)
          res += to_string(signature[i]);
        res += ")";

        return res;
      },
      [](fun_decl, auto fun, auto sort, auto signature) {
        auto res = to_string(fun) + "(" + to_string(signature[0]);
        for(size_t i = 1; i < signature.size(); ++i)
          res += to_string(signature[i]);
        res += ") : " + to_string(sort);

        return res;
      },
      [](sort_decl, auto sort, auto d) {
        auto res = to_string(sort) + " = { " + to_string(d->elements()[0]);
        for(size_t i = 1; i < d->elements().size(); ++i)
          res += to_string(d->elements()[i]);
        res += " }";

        return res;
      }
    );
  }

  std::string to_string(sort s) {
    return s.match(
      [](integer_sort) -> std::string { return "Int"; },
      [](real_sort)    -> std::string { return "Real"; },
      [](named_sort, auto name) { return to_string(name); }
    );
  }

  static inline std::string to_smtlib2(uintptr_t n) {
    return "|" + std::to_string(n) + "|";
  }
  
  static inline std::string to_smtlib2_inner(term<FO> t) {
    using namespace std::literals;
    return t.match(
      [](constant<FO>, auto c) {
        return c.match(
          [](integer, auto v) {
            return std::to_string(v);
          },
          [](real, auto v) {
            return std::to_string(v);
          }
        );
      },
      [](variable x) {
        return to_smtlib2(to_underlying(x.unique_id()));
      },
      [](application<FO> a) {
        std::string s = "(" + to_smtlib2(to_underlying(a.func().unique_id()));

        for(auto arg : a.terms()) {
          s += " " + to_smtlib2_inner(arg);
        }

        s += ")";
        return s;
      }, // LCOV_EXCL_LINE
      [](negative<FO>, auto arg) {
        return fmt::format("(- {})", to_smtlib2_inner(arg));
      },
      [](to_integer<FO>, auto arg) {
        return fmt::format("(to_int {})", to_smtlib2_inner(arg));
      },
      [](to_real<FO>, auto arg) {
        return fmt::format("(to_real {})", to_smtlib2_inner(arg));
      },
      [](addition<FO>, auto left, auto right) {
        return fmt::format(
          "(+ {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      },
      [](subtraction<FO>, auto left, auto right) {
        return fmt::format(
          "(- {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      },
      [](multiplication<FO>, auto left, auto right) {
        return fmt::format(
          "(* {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      },
      [](division<FO>, auto left, auto right) {
        return fmt::format(
          "(/ {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        ); // LCOV_EXCL_LINE
      },
      [](int_division<FO>, auto left, auto right) {
        return fmt::format(
          "(div {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        ); // LCOV_EXCL_LINE
      }
    );
  }

  static inline std::string to_smtlib2_inner(formula<FO> f) {
    return f.match(
      [](boolean, bool b) {
        return b ? "true" : "false";
      },
      [](proposition p) {
        return to_smtlib2(to_underlying(p.unique_id()));
      },
      [](atom<FO> a) {
        std::string s = "(" + to_smtlib2(to_underlying(a.rel().unique_id()));

        for(auto t : a.terms()) {
          s += " " + to_smtlib2_inner(t);
        }

        s += ")";
        return s;
      }, // LCOV_EXCL_LINE
      [](equality<FO> e, auto terms) {
        black_assert(terms.size() > 0);

        std::string args = to_smtlib2_inner(terms[0]);
        for(size_t i = 1; i < terms.size(); ++i)
          args = " " + to_smtlib2_inner(terms[i]);

        return fmt::format(
          "({} {})", e.is<equal<FO>>() ? "=" : "distinct", args
        );
      },
      [](less_than<FO> cmp) {
        return fmt::format(
          "(< {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [](less_than_equal<FO> cmp) {
        return fmt::format(
          "(<= {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [](greater_than<FO> cmp) {
        return fmt::format(
          "(> {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [](greater_than_equal<FO> cmp) {
        return fmt::format(
          "(>= {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [&](quantifier<FO> q) {
        std::string vars;
        for(auto d : q.variables()) {
          vars += fmt::format(
            " ({} {})", 
            to_smtlib2(to_underlying(d.variable().unique_id())), 
            to_string(d.sort())
          );
        }
        vars.erase(vars.begin()); 

        return fmt::format(
          "({} ({}) {})", 
          q.is<exists<FO>>() ? "exists" : "forall",
          vars, to_smtlib2_inner(q.matrix())
        );
      },
      [](negation<FO>, auto op) {
        return fmt::format("(not {})", to_smtlib2_inner(op));
      },
      [](conjunction<FO> c) {
        std::string ops;
        for(auto op : c.operands())
          ops += " " + to_smtlib2_inner(op);
        ops.erase(ops.begin());

        return fmt::format("(and {})", ops);        
      },
      [](disjunction<FO> c) {
        std::string ops;
        for(auto op : c.operands())
          ops += " " + to_smtlib2_inner(op);
        ops.erase(ops.begin());

        return fmt::format("(or {})", ops);        
      },
      //
      // We exclude these two cases from code coverage because to_smtlib2() is
      // only usually called on the encoding formulas which never contain 
      // implications or double implications
      //
      [](implication<FO>, auto left, auto right) { // LCOV_EXCL_LINE
        return fmt::format( // LCOV_EXCL_LINE
          "(=> {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right) // LCOV_EXCL_LINE
        ); // LCOV_EXCL_LINE
      }, // LCOV_EXCL_LINE
      [](iff<FO>, auto left, auto right) { // LCOV_EXCL_LINE
        return fmt::format( // LCOV_EXCL_LINE
          "(= {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right) // LCOV_EXCL_LINE
        ); // LCOV_EXCL_LINE
      } // LCOV_EXCL_LINE
    );
  }

  std::string to_smtlib2(formula<FO> f, scope const& xi) {
    tsl::hopscotch_set<proposition> props;
    tsl::hopscotch_set<variable> vars;
    tsl::hopscotch_set<relation> rels;
    tsl::hopscotch_set<function> funs;

    logic::for_each_child_deep(f, [&](auto child) {
      child.match(
        [&](proposition p) {
          props.insert(p);
        },
        [&](variable x) {
          vars.insert(x);
        },
        [&](atom<FO> a) {
          rels.insert(a.rel());
        },
        [&](application<FO> a) {
          funs.insert(a.func());
        },
        [](otherwise) { }
      );
    });

    std::string smtlib;

    smtlib += "(set-logic ALL)\n\n";

    for(auto p : props) {
      smtlib += 
        fmt::format(
          "(declare-const {} Bool)\n", 
          to_smtlib2(to_underlying(p.unique_id()))
        );
    }
    
    for(auto x : vars) {
      auto s = xi.sort(x);
      
      if(s)
        smtlib += 
          fmt::format(
            "(declare-const {} {})\n", 
            to_smtlib2(to_underlying(x.unique_id())), 
            to_string(*s)
          );
    }
    
    for(auto r : rels) {
      std::optional<std::vector<sort>> signature = xi.signature(r);
      if(!signature)
        continue;

      std::string args = to_string(signature->at(0));
      for(size_t i = 1; i < signature->size(); ++i) {
        args += " " + to_string(signature->at(i));
      }

      smtlib += fmt::format(
        "(declare-fun {} ({}) Bool)\n", 
        to_smtlib2(to_underlying(r.unique_id())), 
        args
      );
    }
    
    for(auto fun : funs) {
      std::optional<std::vector<sort>> signature = xi.signature(fun);
      std::optional<sort> result = xi.sort(fun);

      if(!signature || !result)
        continue;

      std::string args = to_string(signature->at(0));
      for(size_t i = 1; i < signature->size(); ++i) {
        args += " " + to_string(signature->at(i));
      }

      smtlib += fmt::format(
        "(declare-fun {} ({}) {})\n", 
        to_smtlib2(to_underlying(fun.unique_id())), 
        args,
        to_string(*result)
      );
    }

    smtlib += "\n";

    f.match(
      [&](conjunction<FO> c) {
        for(auto op : c.operands()) {
          smtlib += fmt::format("(assert {})\n\n", to_smtlib2_inner(op));
        }
      },
      [&](otherwise) {
        smtlib += fmt::format("(assert {})\n", to_smtlib2_inner(f));
      }
    );

    smtlib += "(check-sat)\n";

    return smtlib;
  }
}

