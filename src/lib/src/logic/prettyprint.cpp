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

#include <iostream>

namespace black_internal::logic
{

  static
  bool does_need_parens(formula parent, formula arg) {
    bool parens = false;
    arg.match(
      [&](binary) {
        parens = 
        (!parent.is<conjunction>() && !parent.is<disjunction>())
          || (parent.node_type() != arg.node_type());
      },
      [&](comparison) {
        parens = true;
      },
      [&](quantifier) {
        parens = true;
      },
      [&](otherwise) { }
    );
    
    return parens;
  }

  static
  std::string to_string(unary::type t) {
    switch(t) {
      case unary::type::negation:
        return "!";
      case unary::type::tomorrow:
        return "X";
      case unary::type::w_tomorrow:
        return "wX";
      case unary::type::yesterday:
        return "Y";
      case unary::type::w_yesterday:
        return "Z";
      case unary::type::always:
        return "G";
      case unary::type::eventually:
        return "F";
      case unary::type::once:
        return "O";
      case unary::type::historically:
        return "H";
    }
    black_unreachable();
  }

  static
  std::string to_string(binary::type t) {
    switch(t) {
      case binary::type::conjunction:
        return "&";
      case binary::type::disjunction:
        return "|";
      case binary::type::implication:
        return "->";
      case binary::type::iff:        
        return "<->";
      case binary::type::until:      
        return "U";
      case binary::type::release:    
        return "R";
      case binary::type::w_until:    
        return "W";
      case binary::type::s_release:  
        return "M";
      case binary::type::since:      
        return "S";
      case binary::type::triggered:  
        return "T";
    }
    black_unreachable();
  }

  static
  std::string to_string(equality::type t, bool binary) {
    switch(t) {
      case equality::type::equal:
        return binary ? "=" : "equal";
      case equality::type::distinct:
        return binary ? "!=" : "distinct";
    }
    black_unreachable();
  }

  static
  std::string to_string(comparison::type t) {
    switch(t) {
      case comparison::type::less_than:
        return "<";
      case comparison::type::less_than_equal:
        return "<=";
      case comparison::type::greater_than:
        return ">";
      case comparison::type::greater_than_equal:
        return ">=";
    }
    black_unreachable();
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
  std::string term_parens(term t) {
    return t.match(
      [&](variable) {
        return to_string(t);
      },
      [&](constant) {
        return to_string(t);
      },
      [&](otherwise) {
        return "(" + to_string(t) + ")";
      }
    );
  }

  std::string to_string(term t)
  {
    using namespace std::literals;
    using namespace black_internal;

    return t.match(
      [&](constant c) {
        return to_string(c.value());
      },
      [&](variable x) {
        return escape(to_string(x.name()));
      },
      [&](star) {
        return "*";
      },
      [&](application a) {
        std::string result = 
          escape(to_string(a.func().name())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [&](negative, auto arg) {
        return fmt::format("-({})", to_string(arg));
      },
      [&](to_integer, auto arg) {
        return fmt::format("to_int({})", to_string(arg));
      },
      [&](to_real, auto arg) {
        return fmt::format("to_real({})", to_string(arg));
      },
      [&](next, auto arg) {
        return fmt::format("next({})", to_string(arg));
      },
      [&](wnext, auto arg) {
        return fmt::format("wnext({})", to_string(arg));
      },
      [&](prev, auto arg) {
        return fmt::format("prev({})", to_string(arg));
      },
      [&](wprev, auto arg) {
        return fmt::format("wprev({})", to_string(arg));
      },
      [&](addition, auto left, auto right) {
        return fmt::format("{} + {}", term_parens(left), term_parens(right));
      },
      [&](subtraction, auto left, auto right) {
        return fmt::format("{} - {}", term_parens(left), term_parens(right));
      },
      [&](multiplication, auto left, auto right) {
        return fmt::format("{} * {}", term_parens(left), term_parens(right));
      },
      [&](division, auto left, auto right) {
        return fmt::format("{} / {}", term_parens(left), term_parens(right));
      },
      [&](int_division, auto left, auto right) {
        return fmt::format("{} div {}", term_parens(left), term_parens(right));
      }
    );
  }

  std::string to_string(formula f)
  {
    using namespace std::literals;
    using namespace ::black_internal;

    return f.match(
      [&](proposition p) {
        return escape(to_string(p.name()));
      },
      [&](atom a) {
        std::string result = 
          escape(to_string(a.rel().name())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [](equality e, auto terms) {
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
      [](comparison c, auto left, auto right) {
        return fmt::format(
          "{} {} {}", 
          to_string(left), to_string(c.node_type()), to_string(right)
        );
      },
      [](modality m, term standpoint, formula argument) {
        char open = m.node_type() == modality::type::box ? '[' : '<';
        char close = m.node_type() == modality::type::box ? ']' : '>';

        std::string spname = standpoint.match(
          [](star) { return "*"; },
          [](variable, auto name) { return to_string(name); }
        );

        bool parens = does_need_parens(m, argument);

        return fmt::format(
          "{}{}{}{}", open, spname, close, parens_if_needed(argument, parens)
        );
      },
      [](quantifier q) {
        std::string qs = q.node_type() == quantifier::type::exists ?
          "exists " : "forall ";

        bool parens = q.matrix().is<binary>();

        for(var_decl d : q.variables()) {
          qs += 
            '(' + to_string(d.variable()) + " : " + to_string(d.sort()) + ") ";
        }

        return fmt::format("{}. {}", qs, parens_if_needed(q.matrix(), parens));
      }, // LCOV_EXCL_LINE
      [](boolean, bool b) {
        return b ? "True" : "False";
      },
      [](negation n, auto arg) {
        bool needs_parens = does_need_parens(n, arg);
        return fmt::format("{}{}", 
          to_string(unary::type::negation), 
          parens_if_needed(arg, needs_parens)
        );
      },
      [](unary u, auto arg) {
        bool needs_parens = does_need_parens(u, arg);
        return fmt::format("{}{}{}",
                            to_string(u.node_type()),
                            needs_parens ? "" : " ",
                            parens_if_needed(arg, needs_parens));
      },
      [](binary b, auto left, auto right) {
        return
          fmt::format("{} {} {}",
                      parens_if_needed(left, does_need_parens(b, left)),
                      to_string(b.node_type()),
                      parens_if_needed(right, does_need_parens(b, right)));
      }
    );
  }

  std::string to_string(symbol s) {
    return s.match(
      [](relation r) {
        return to_string(r.name());
      },
      [](function f) {
        return to_string(f.name());
      }
    );
  }

  std::string to_string(number n) {
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
  
  static inline std::string to_smtlib2_inner(term t) {
    using namespace std::literals;
    return t.match(
      [](constant, auto c) {
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
      [](application a) {
        std::string s = "(" + to_smtlib2(to_underlying(a.func().unique_id()));

        for(auto arg : a.terms()) {
          s += " " + to_smtlib2_inner(arg);
        }

        s += ")";
        return s;
      }, // LCOV_EXCL_LINE
      [](negative, auto arg) {
        return fmt::format("(- {})", to_smtlib2_inner(arg));
      },
      [](to_integer, auto arg) {
        return fmt::format("(to_int {})", to_smtlib2_inner(arg));
      },
      [](to_real, auto arg) {
        return fmt::format("(to_real {})", to_smtlib2_inner(arg));
      },
      [](addition, auto left, auto right) {
        return fmt::format(
          "(+ {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      },
      [](subtraction, auto left, auto right) {
        return fmt::format(
          "(- {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      },
      [](multiplication, auto left, auto right) {
        return fmt::format(
          "(* {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        );
      },
      [](division, auto left, auto right) {
        return fmt::format(
          "(/ {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        ); // LCOV_EXCL_LINE
      },
      [](int_division, auto left, auto right) {
        return fmt::format(
          "(div {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right)
        ); // LCOV_EXCL_LINE
      }
    );
  }

  static inline std::string to_smtlib2_inner(formula f) {
    return f.match(
      [](boolean, bool b) {
        return b ? "true" : "false";
      },
      [](proposition p) {
        return to_smtlib2(to_underlying(p.unique_id()));
      },
      [](atom a) {
        std::string s = "(" + to_smtlib2(to_underlying(a.rel().unique_id()));

        for(auto t : a.terms()) {
          s += " " + to_smtlib2_inner(t);
        }

        s += ")";
        return s;
      }, // LCOV_EXCL_LINE
      [](equality e, auto terms) {
        black_assert(terms.size() > 0);

        std::string args = to_smtlib2_inner(terms[0]);
        for(size_t i = 1; i < terms.size(); ++i)
          args = " " + to_smtlib2_inner(terms[i]);

        return fmt::format(
          "({} {})", e.is<equal>() ? "=" : "distinct", args
        );
      },
      [](less_than cmp) {
        return fmt::format(
          "(< {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [](less_than_equal cmp) {
        return fmt::format(
          "(<= {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [](greater_than cmp) {
        return fmt::format(
          "(> {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [](greater_than_equal cmp) {
        return fmt::format(
          "(>= {} {})", 
          to_smtlib2_inner(cmp.left()), to_smtlib2_inner(cmp.right())
        );
      },
      [&](quantifier q) {
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
          q.is<exists>() ? "exists" : "forall",
          vars, to_smtlib2_inner(q.matrix())
        );
      },
      [](negation, auto op) {
        return fmt::format("(not {})", to_smtlib2_inner(op));
      },
      [](conjunction c) {
        std::string ops;
        for(auto op : operands(c))
          ops += " " + to_smtlib2_inner(op);
        ops.erase(ops.begin());

        return fmt::format("(and {})", ops);        
      },
      [](disjunction c) {
        std::string ops;
        for(auto op : operands(c))
          ops += " " + to_smtlib2_inner(op);
        ops.erase(ops.begin());

        return fmt::format("(or {})", ops);        
      },
      //
      // We exclude these two cases from code coverage because to_smtlib2() is
      // only usually called on the encoding formulas which never contain 
      // implications or double implications
      //
      [](implication, auto left, auto right) { // LCOV_EXCL_LINE
        return fmt::format( // LCOV_EXCL_LINE
          "(=> {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right) // LCOV_EXCL_LINE
        ); // LCOV_EXCL_LINE
      }, // LCOV_EXCL_LINE
      [](iff, auto left, auto right) { // LCOV_EXCL_LINE
        return fmt::format( // LCOV_EXCL_LINE
          "(= {} {})", to_smtlib2_inner(left), to_smtlib2_inner(right) // LCOV_EXCL_LINE
        ); // LCOV_EXCL_LINE
      } // LCOV_EXCL_LINE
    );
  }

  std::string to_smtlib2(formula f, scope const& xi) {
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
        [&](atom a) {
          rels.insert(a.rel());
        },
        [&](application a) {
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
      [&](conjunction c) {
        for(auto op : operands(c)) {
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

