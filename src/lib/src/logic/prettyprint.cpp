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
#include <black/logic/past_remover.hpp>

#include <fmt/format.h>

namespace black::internal 
{
  namespace 
  {
    bool does_need_parens(formula parent, formula arg) {
      bool parens = false;
      if(arg.is<binary>()) {
        parens = (!parent.is<conjunction>() && !parent.is<disjunction>())
              || (parent.formula_type() != arg.formula_type());
      }
      if(arg.is<atom>() && arg.to<atom>()->rel().known_type().has_value())
        parens = true;
      if(arg.is<quantifier>())
        parens = true;

      return parens;
    }

    bool does_need_parens(term /* parent */, term arg) {
      return arg.match(
        [](application a) {
          return a.func().known_type().has_value();
        },
        [](otherwise) {
          return false;
        }
      );
    }

    template<typename T>
    std::string parens_if_needed(T t, bool needs_parens) {
      return needs_parens ? "(" + to_string(t) + ")" : to_string(t);
    }
  }

  std::string to_string(term t)
  {
    using namespace std::literals;
    return t.match(
      [&](constant c) {
        if(std::holds_alternative<int64_t>(c.value()))
          return fmt::format("{}", std::get<int64_t>(c.value()));
        else
          return fmt::format("{}", std::get<double>(c.value()));
      },
      [&](variable x) {
        if(auto name = x.label<std::string>(); name.has_value())
          return *name;
        if(auto fname = x.label<std::pair<term,size_t>>(); fname.has_value()) {
          return
            fmt::format("<{},{}>", to_string(fname->first), fname->second);
        }
        return fmt::format("<{:x}>", to_underlying(term{x}.unique_id())); // LCOV_EXCL_LINE
      },
      [&](application a) {
        if(auto t2 = a.func().known_type(); t2) {
          if(t2 == function::type::negation)
            return fmt::format("-{}", to_string(a.arguments()[0]));
          else {
            term lhs = a.arguments()[0];
            term rhs = a.arguments()[1];
            return fmt::format("{} {} {}", 
              parens_if_needed(lhs, does_need_parens(a, lhs)),
              a.func().name(),
              parens_if_needed(rhs, does_need_parens(a, rhs))
            );
          }
        }
        std::string result = 
          a.func().name() + "(" + to_string(a.arguments()[0]);
        for(size_t i = 1; i < a.arguments().size(); ++i) {
          result += ", " + to_string(a.arguments()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [&](next n) {
        return fmt::format("next({})", to_string(n.argument()));
      },
      [&](wnext n) {
        return fmt::format("wnext({})", to_string(n.argument()));
      }
    );
  }

  std::string to_string(formula f)
  {
    using namespace std::literals;
    return f.match(
      [&](proposition p) {
        if(auto name = p.label<std::string>(); name.has_value())
          return *name;
        if(auto fname = p.label<std::pair<formula,size_t>>(); fname.has_value())
          return
            fmt::format("<{},{}>", to_string(fname->first), fname->second); // LCOV_EXCL_LINE
        if(auto fname = p.label<past_label>(); fname.has_value())
          return fmt::format("<{}>", to_string(fname->formula)); // LCOV_EXCL_LINE
        else
          return fmt::format("<{:x}>", to_underlying(formula{p}.unique_id()));
      },
      [&](atom a) {
        if(auto t = a.rel().known_type(); t)
          return fmt::format(
            "{} {} {}", 
            to_string(a.terms()[0]),
            a.rel().name(), 
            to_string(a.terms()[1])
          );
        
        std::string result = 
          a.rel().name() + "(" + to_string(a.terms()[0]);
        for(size_t i = 1; i < a.terms().size(); ++i) {
          result += ", " + to_string(a.terms()[i]);
        }
        result += ")";

        return result;
      }, // LCOV_EXCL_LINE
      [](quantifier_block q) {
        std::string qs = q.quantifier_type() == quantifier::type::exists ?
          "exists " : "forall ";

        bool parens = q.matrix().is<binary>();

        for(variable v : q.vars())
          qs += to_string(v) + ' ';

        return fmt::format("{}. {}", qs, parens_if_needed(q.matrix(), parens));
      }, // LCOV_EXCL_LINE
      [](boolean b) {
        return b.value() ? "True" : "False";
      },
      [](negation n) {
        auto arg = n.operand();
        bool needs_parens = does_need_parens(n, arg);
        return fmt::format("!{}", parens_if_needed(arg, needs_parens));
      },
      [](unary u) {
        auto arg = u.operand();
        bool needs_parens = does_need_parens(u, arg);
        return fmt::format("{}{}{}",
                            to_string(u.formula_type()),
                            needs_parens ? "" : " ",
                            parens_if_needed(arg, needs_parens));
      },
      [](binary b) {
        auto lhs = b.left(), rhs = b.right();
        return
          fmt::format("{} {} {}",
                      parens_if_needed(lhs, does_need_parens(b, lhs)),
                      to_string(b.formula_type()),
                      parens_if_needed(rhs, does_need_parens(b, rhs)));
      }
    );
  }
}

