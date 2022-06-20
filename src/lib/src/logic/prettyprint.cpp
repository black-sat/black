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
      [](binary<LTLPFO>::type::conjunction) { return "&&"; },
      [](binary<LTLPFO>::type::disjunction) { return "||"; },
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
      i = 1;
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
          [](zero) { return "0"s; },
          [](one) { return "1"s; },
          [](integer, int64_t value) {
            return fmt::format("{}", value);
          },
          [](real, double value) {
            return fmt::format("{}", value);
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
      },
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
        return fmt::format("!{}", parens_if_needed(arg, needs_parens));
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
}

