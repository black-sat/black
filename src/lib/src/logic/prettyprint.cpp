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

#include <fmt/format.h>

using namespace black::LTLPFO;

namespace black::internal 
{
  static
  bool does_need_parens(formula parent, formula arg) {
    bool parens = false;
    arg.match(
      [&](binary) {
        parens = (!parent.is<conjunction>() && !parent.is<disjunction>())
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
    return t.match(
      [](type_value<syntax_element::negation>)     { return "!"; },
      [](type_value<syntax_element::tomorrow>)     { return "X"; },
      [](type_value<syntax_element::w_tomorrow>)   { return "wX"; },
      [](type_value<syntax_element::yesterday>)    { return "Y"; },
      [](type_value<syntax_element::w_yesterday>)  { return "Z"; },
      [](type_value<syntax_element::always>)       { return "G"; },
      [](type_value<syntax_element::eventually>)   { return "F"; },
      [](type_value<syntax_element::once>)         { return "O"; },
      [](type_value<syntax_element::historically>) { return "H"; }
    );
  }

  static
  std::string to_string(binary::type t) {
    return t.match(
      [](type_value<syntax_element::conjunction>) { return "&&"; },
      [](type_value<syntax_element::disjunction>) { return "||"; },
      [](type_value<syntax_element::implication>) { return "->"; },
      [](type_value<syntax_element::iff>)         { return "<->"; },
      [](type_value<syntax_element::until>)       { return "U"; },
      [](type_value<syntax_element::release>)     { return "R"; },
      [](type_value<syntax_element::w_until>)     { return "W"; },
      [](type_value<syntax_element::s_release>)   { return "M"; },
      [](type_value<syntax_element::since>)       { return "S"; },
      [](type_value<syntax_element::triggered>)   { return "T"; }
    );
  }

  template<typename T>
  std::string parens_if_needed(T t, bool needs_parens) {
    return needs_parens ? "(" + to_string(t) + ")" : to_string(t);
  }

  static
  std::string escape(std::string s) {
    // TODO: after migrating the lexer
    // if(s.empty())
    //   return "{}";

    // if(lexer::is_keyword(s)) {
    //   return "{" + s + "}";
    // }
    
    // bool escaped = false;
    // if(!lexer::is_initial_identifier_char(s[0]))
    //   escaped = true;
    // size_t i = 0;
    // if(s[0] == '}' || s[0] == '\\') {
    //   s.insert(0, 1, '\\');
    //   i = 1;
    // }

    // while(i < s.size()) {
    //   if(!lexer::is_identifier_char(s[i]))
    //     escaped = true;
    //   if(s[i] == '}' || s[i] == '\\') {
    //     s.insert(i, 1, '\\');
    //     i++;
    //   }
    //   i++;
    // }

    // if(escaped)
    //   return "{" + s + "}";
    return s;
  }
}

namespace black
{
  std::string to_string(term t)
  {
    using namespace std::literals;
    using namespace black::internal;

    return t.match(
      [&](constant c) {
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
        return escape(to_string(x.label()));
      },
      [&](application a) {
        std::string result = 
          escape(to_string(a.func().label())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      },
      [&](negative, term arg) {
        return fmt::format("-({})", to_string(arg));
      },
      [&](next, term arg) {
        return fmt::format("next({})", to_string(arg));
      },
      [&](wnext, term arg) {
        return fmt::format("wnext({})", to_string(arg));
      },
      [&](prev, term arg) {
        return fmt::format("prev({})", to_string(arg));
      },
      [&](wprev, term arg) {
        return fmt::format("wprev({})", to_string(arg));
      },
      [&](addition, term left, term right) {
        return fmt::format("({}) + ({})", to_string(left), to_string(right));
      },
      [&](subtraction, term left, term right) {
        return fmt::format("({}) - ({})", to_string(left), to_string(right));
      },
      [&](multiplication, term left, term right) {
        return fmt::format("({}) * ({})", to_string(left), to_string(right));
      },
      [&](division, term left, term right) {
        return fmt::format("({}) / ({})", to_string(left), to_string(right));
      }
    );
  }

  std::string to_string(formula f)
  {
    using namespace std::literals;
    using namespace ::black::internal;

    return f.match(
      [&](proposition p) {
        return escape(to_string(p.label()));
      },
      [&](atom a) {
        std::string result = 
          escape(to_string(a.rel().label())) + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [](quantifier_block q) {
        std::string qs = q.node_type() == quantifier::type::exists ?
          "exists " : "forall ";

        bool parens = q.matrix().is<binary>();

        for(variable v : q.variables())
          qs += to_string(v) + ' ';

        return fmt::format("{}. {}", qs, parens_if_needed(q.matrix(), parens));
      }, // LCOV_EXCL_LINE
      [](boolean, bool b) {
        return b ? "True" : "False";
      },
      [](negation n, formula arg) {
        bool needs_parens = does_need_parens(n, arg);
        return fmt::format("!{}", parens_if_needed(arg, needs_parens));
      },
      [](unary u, formula arg) {
        bool needs_parens = does_need_parens(u, arg);
        return fmt::format("{}{}{}",
                            to_string(u.node_type()),
                            needs_parens ? "" : " ",
                            parens_if_needed(arg, needs_parens));
      },
      [](binary b, formula left, formula right) {
        return
          fmt::format("{} {} {}",
                      parens_if_needed(left, does_need_parens(b, left)),
                      to_string(b.node_type()),
                      parens_if_needed(right, does_need_parens(b, right)));
      }
    );
  }
}


