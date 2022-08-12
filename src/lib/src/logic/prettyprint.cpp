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
  bool does_need_parens(formula<LTLPFO> parent, formula<LTLPFO> arg) {
    bool parens = false;
    arg.match(
      [&](binary<LTLPFO>) {
        parens = 
        (!parent.is<conjunction<LTLPFO>>() && !parent.is<disjunction<LTLPFO>>())
          || (parent.node_type() != arg.node_type());
      },
      [&](comparison<LTLPFO>) {
        parens = true;
      },
      [&](quantifier<LTLPFO>) {
        parens = true;
      },
      [&](otherwise) { }
    );
    
    return parens;
  }

  static
  std::string to_string(unary<LTLPFO>::type t) {
    return t.match(
      [](unary<LTLPFO>::type::negation)     { return "!"; },
      [](unary<LTLPFO>::type::tomorrow)     { return "X"; },
      [](unary<LTLPFO>::type::w_tomorrow)   { return "wX"; },
      [](unary<LTLPFO>::type::yesterday)    { return "Y"; },
      [](unary<LTLPFO>::type::w_yesterday)  { return "Z"; },
      [](unary<LTLPFO>::type::always)       { return "G"; },
      [](unary<LTLPFO>::type::eventually)   { return "F"; },
      [](unary<LTLPFO>::type::once)         { return "O"; },
      [](unary<LTLPFO>::type::historically) { return "H"; }
    );
  }

  static
  std::string to_string(binary<LTLPFO>::type t) {
    return t.match(
      [](binary<LTLPFO>::type::conjunction) { return "&"; },
      [](binary<LTLPFO>::type::disjunction) { return "|"; },
      [](binary<LTLPFO>::type::implication) { return "->"; },
      [](binary<LTLPFO>::type::iff)         { return "<->"; },
      [](binary<LTLPFO>::type::until)       { return "U"; },
      [](binary<LTLPFO>::type::release)     { return "R"; },
      [](binary<LTLPFO>::type::w_until)     { return "W"; },
      [](binary<LTLPFO>::type::s_release)   { return "M"; },
      [](binary<LTLPFO>::type::since)       { return "S"; },
      [](binary<LTLPFO>::type::triggered)   { return "T"; }
    );
  }

  static
  std::string to_string(comparison<LTLPFO>::type t) {
    return t.match(
      [](comparison<LTLPFO>::type::equal)              { return "="; },
      [](comparison<LTLPFO>::type::not_equal)          { return "!="; },
      [](comparison<LTLPFO>::type::less_than)          { return "<"; },
      [](comparison<LTLPFO>::type::less_than_equal)    { return "<="; },
      [](comparison<LTLPFO>::type::greater_than)       { return ">"; },
      [](comparison<LTLPFO>::type::greater_than_equal) { return ">="; }
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

  std::string to_string(term<LTLPFO> t)
  {
    using namespace std::literals;
    using namespace black_internal;

    return t.match(
      [&](constant<LTLPFO> c) {
        return c.value().match(
          [](zero z) { 
            if(z.sigma()->default_sort().is<integer_sort>())
              return "0"s; 
            return "0.0"s;
          },
          [](one o) { 
            if(o.sigma()->default_sort().is<integer_sort>())
              return "1"s; 
            return "1.0"s;
          },
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
      },
      [&](variable x) {
        return escape(to_string(x.name()));
      },
      [&](application<LTLPFO> a) {
        std::string result = 
          escape(to_string(a.func().name())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [&](negative<LTLPFO>, auto arg) {
        return fmt::format("-({})", to_string(arg));
      },
      [&](next<LTLPFO>, auto arg) {
        return fmt::format("next({})", to_string(arg));
      },
      [&](wnext<LTLPFO>, auto arg) {
        return fmt::format("wnext({})", to_string(arg));
      },
      [&](prev<LTLPFO>, auto arg) {
        return fmt::format("prev({})", to_string(arg));
      },
      [&](wprev<LTLPFO>, auto arg) {
        return fmt::format("wprev({})", to_string(arg));
      },
      [&](addition<LTLPFO>, auto left, auto right) {
        return fmt::format("({}) + ({})", to_string(left), to_string(right));
      },
      [&](subtraction<LTLPFO>, auto left, auto right) {
        return fmt::format("({}) - ({})", to_string(left), to_string(right));
      },
      [&](multiplication<LTLPFO>, auto left, auto right) {
        return fmt::format("({}) * ({})", to_string(left), to_string(right));
      },
      [&](division<LTLPFO>, auto left, auto right) {
        return fmt::format("({}) / ({})", to_string(left), to_string(right));
      }
    );
  }

  std::string to_string(formula<LTLPFO> f)
  {
    using namespace std::literals;
    using namespace ::black_internal;

    return f.match(
      [&](proposition p) {
        return escape(to_string(p.name()));
      },
      [&](atom<LTLPFO> a) {
        std::string result = 
          escape(to_string(a.rel().name())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [](comparison<LTLPFO> c, auto left, auto right) {
        return fmt::format(
          "{} {} {}", 
          to_string(left), to_string(c.node_type()), to_string(right)
        );
      },
      [](quantifier_block<LTLPFO> q) {
        std::string qs = q.node_type() == quantifier<LTLPFO>::type::exists{} ?
          "exists " : "forall ";

        bool parens = q.matrix().is<binary<LTLPFO>>();

        for(variable v : q.variables())
          qs += to_string(v) + ' ';

        return fmt::format("{}. {}", qs, parens_if_needed(q.matrix(), parens));
      }, // LCOV_EXCL_LINE
      [](boolean, bool b) {
        return b ? "True" : "False";
      },
      [](negation<LTLPFO> n, auto arg) {
        bool needs_parens = does_need_parens(n, arg);
        return fmt::format("{}{}", 
          to_string(unary<LTLPFO>::type::negation{}), 
          parens_if_needed(arg, needs_parens)
        );
      },
      [](unary<LTLPFO> u, auto arg) {
        bool needs_parens = does_need_parens(u, arg);
        return fmt::format("{}{}{}",
                            to_string(u.node_type()),
                            needs_parens ? "" : " ",
                            parens_if_needed(arg, needs_parens));
      },
      [](binary<LTLPFO> b, auto left, auto right) {
        return
          fmt::format("{} {} {}",
                      parens_if_needed(left, does_need_parens(b, left)),
                      to_string(b.node_type()),
                      parens_if_needed(right, does_need_parens(b, right)));
      }
    );
  }

  std::string to_string(relation r) {
    return to_string(r.name());
  }
  
  std::string to_string(function f) {
    return to_string(f.name());
  }

  std::string to_string(sort s) {
    return s.match(
      [](integer_sort) { return "integers"; },
      [](real_sort)    { return "reals"; },
      [](otherwise)    -> const char *{ black_unreachable(); } // LCOV_EXCL_LINE
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
          [](zero) { return "0"s; },
          [](one) { return "1"s; },
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
      },
      [](negative<FO>, auto arg) {
        return fmt::format("(- {})", to_smtlib2_inner(arg));
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
        if(left.sigma()->default_sort().template is<integer_sort>())
          return fmt::format(
            "(div {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
          );
        return fmt::format(
          "(/ {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      }
    );
  }

  static inline std::string to_smtlib2_inner(formula<FO> f) {
    std::string type = f.sigma()->default_sort().match(
      [](integer_sort) { return "Int"; },
      [](real_sort) { return "Real"; },
      [](otherwise) -> const char *{ black_unreachable(); } // LCOV_EXCL_LINE
    );

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
      },
      [](equal<FO> cmp) {
        return fmt::format(
          "(= {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [](not_equal<FO> cmp) {
        return fmt::format(
          "(distinct {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
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
        for(auto x : q.block().variables()) {
          vars += fmt::format(
            " ({} {})", to_smtlib2(to_underlying(x.unique_id())), type
          );
        }
        vars.erase(vars.begin()); 

        return fmt::format(
          "({} ({}) {})", 
          q.is<exists<FO>>() ? "exists" : "forall",
          vars, to_smtlib2_inner(q.block().matrix())
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
      [](implication<FO>, auto left, auto right) {
        return fmt::format(
          "(=> {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      },
      [](iff<FO>, auto left, auto right) {
        return fmt::format(
          "(= {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      }
    );
  }

  static inline
  void smtlib2_collect_vars(
    auto h, tsl::hopscotch_set<variable> &vars,
    tsl::hopscotch_set<variable> &scope
  ) {
    h.match(
      [&](quantifier<FO> q) {
        tsl::hopscotch_set<variable> new_scope;
        for(auto var : q.block().variables()) {
          new_scope.insert(var);
        }
        smtlib2_collect_vars(q.block().matrix(), vars, new_scope);
      },
      [&](variable x) {
        if(scope.find(x) != scope.end())
          vars.insert(x);
      },
      [&](otherwise) {
        for_each_child(h, [&](auto child) {
          smtlib2_collect_vars(child, vars, scope);
        });
      }
    );
  }

  std::string to_smtlib2(formula<FO> f) {
    tsl::hopscotch_set<proposition> props;
    tsl::hopscotch_set<variable> vars;
    tsl::hopscotch_map<relation, int> rels;
    tsl::hopscotch_map<function, int> funs;

    logic::for_each_child_deep(f, [&](auto child) {
      child.match(
        [&](proposition p) {
          props.insert(p);
        },
        [&](atom<FO> a) {
          rels.insert({a.rel(), a.terms().size()});
        },
        [&](application<FO> a) {
          funs.insert({a.func(), a.terms().size()});
        },
        [](otherwise) { }
      );
    });

    tsl::hopscotch_set<variable> empty;
    smtlib2_collect_vars(f, vars, empty);

    std::string s = f.sigma()->default_sort().match(
      [](integer_sort) { return "Int"; },
      [](real_sort) { return "Real"; },
      [](otherwise) -> const char *{ black_unreachable(); } // LCOV_EXCL_LINE
    );

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
      smtlib += 
        fmt::format(
          "(declare-const {} {})\n", 
          to_smtlib2(to_underlying(x.unique_id())), 
          s
        );
    }
    
    for(auto [r,arity] : rels) {
      std::string args = s;
      for(int i = 1; i < arity; ++i) {
        args += " " + s;
      }

      smtlib += fmt::format(
        "(declare-fun {} ({}) Bool)\n", 
        to_smtlib2(to_underlying(r.unique_id())), 
        args
      );
    }
    
    for(auto [fun,arity] : funs) {
      std::string args = s;
      for(int i = 1; i < arity; ++i) {
        args += " " + s;
      }

      smtlib += fmt::format(
        "(declare-fun {} ({}) {})\n", 
        to_smtlib2(to_underlying(fun.unique_id())), 
        args,
        s
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

