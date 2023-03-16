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

#include <tsl/hopscotch_set.h>
#include <tsl/hopscotch_map.h>

#include <fmt/format.h>

namespace black::logic::internal
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
  std::string to_string(equality<LTLPFO>::type t, bool binary) {
    if(binary)
      return t.match(
        [](equality<LTLPFO>::type::equal)    { return "="; },
        [](equality<LTLPFO>::type::distinct) { return "!="; }
      );
    return t.match(
      [](equality<LTLPFO>::type::equal)    { return "equal"; },
      [](equality<LTLPFO>::type::distinct) { return "distinct"; }
    );
  }

  static
  std::string to_string(comparison<LTLPFO>::type t) {
    return t.match(
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
    return s;
  }

  static
  std::string term_parens(term<LTLPFO> t) {
    return t.match(
      [&](variable) {
        return to_string(t);
      },
      [&](constant<LTLPFO>) {
        return to_string(t);
      },
      [&](otherwise) {
        return "(" + to_string(t) + ")";
      }
    );
  }

  std::string to_string(term<LTLPFO> t)
  {
    using namespace std::literals;
    using namespace black::support::internal;
    using namespace black::logic::internal;

    return t.match(
      [&](constant<LTLPFO> c) {
        return to_string(c.value());
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
      [&](to_integer<LTLPFO>, auto arg) {
        return fmt::format("to_int({})", to_string(arg));
      },
      [&](to_real<LTLPFO>, auto arg) {
        return fmt::format("to_real({})", to_string(arg));
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
        return fmt::format("{} + {}", term_parens(left), term_parens(right));
      },
      [&](subtraction<LTLPFO>, auto left, auto right) {
        return fmt::format("{} - {}", term_parens(left), term_parens(right));
      },
      [&](multiplication<LTLPFO>, auto left, auto right) {
        return fmt::format("{} * {}", term_parens(left), term_parens(right));
      },
      [&](division<LTLPFO>, auto left, auto right) {
        return fmt::format("{} / {}", term_parens(left), term_parens(right));
      },
      [&](int_division<LTLPFO>, auto left, auto right) {
        return fmt::format("{} div {}", term_parens(left), term_parens(right));
      }
    );
  }

  std::string to_string(formula<LTLPFO> f)
  {
    using namespace std::literals;
    using namespace ::black::support::internal;
    using namespace ::black::logic::internal;

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
      [](equality<LTLPFO> e, auto terms) {
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
      [](comparison<LTLPFO> c, auto left, auto right) {
        return fmt::format(
          "{} {} {}", 
          to_string(left), to_string(c.node_type()), to_string(right)
        );
      },
      [](quantifier<LTLPFO> q) {
        std::string qs = q.node_type() == quantifier<LTLPFO>::type::exists{} ?
          "exists " : "forall ";

        bool parens = q.matrix().is<binary<LTLPFO>>();

        for(var_decl d : q.variables()) {
          qs += 
            '(' + to_string(d.variable()) + " : " + to_string(d.sort()) + ") ";
        }

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

  std::string to_string(symbol<LTLPFO> s) {
    return s.match(
      [](relation r) {
        return to_string(r.name());
      },
      [](function f) {
        return to_string(f.name());
      }
    );
  }

  std::string to_string(number<LTLPFO> n) {
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
}

