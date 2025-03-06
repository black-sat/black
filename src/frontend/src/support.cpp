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

#include <black/frontend/support.hpp>

namespace black::frontend 
{
  std::string system_error_string(int errnum)
  {
    char buf[255];
    const int buflen = sizeof(buf);

    // strerror has different signatures on different systems
    #ifdef _GNU_SOURCE
        // GNU-specific version
        return strerror_r(errnum, buf, buflen);
    #elif defined(_MSC_VER)
        // Microsoft-specific version
        strerror_s(buf, buflen, errnum);
        return buf;
    #else
        // XSI-compliant systems, including MacOS and FreeBSD
        strerror_r(errnum, buf, buflen);
        return buf;
    #endif
  }

  std::ifstream open_file(std::string const&path) {
    std::ifstream file{path, std::ios::in};

    if(!file)
      io::fatal(status_code::filesystem_error,
        "Unable to open file `{}`: {}",
        path, system_error_string(errno)
      );

    return file;
  }

  std::ofstream open_out_file(std::string const&path) {
    std::ofstream file{path, std::ios::out};

    // Errors in opening a file for writing are difficult to test, and this
    // function  as of now is only used in a debugging feature, so we ignore
    // these lines. The code is identical to open_file(), which is tested.
    if(!file) // LCOV_EXCL_LINE
      io::fatal(status_code::filesystem_error, // LCOV_EXCL_LINE
        "Unable to open file `{}`: {}", // LCOV_EXCL_LINE
        path, system_error_string(errno) // LCOV_EXCL_LINE
      ); // LCOV_EXCL_LINE

    return file;
  }

  std::function<void(std::string)> 
  formula_syntax_error_handler(std::optional<std::string> const&path)
  {
    auto readable_syntax_error = [path](auto error) {
      io::fatal(status_code::syntax_error, 
                "syntax error: {}: {}\n", 
                path ? *path : "<stdin>", error); // LCOV_EXCL_LINE
    };

    auto json_syntax_error = [](auto error) {
      io::fatal(status_code::syntax_error,
        "{{\n"
        "    \"result\": \"ERROR\",\n"
        "    \"error\": \"{}\"\n"
        "}}", error);
    };

    if(!cli::output_format || cli::output_format == "readable")
      return readable_syntax_error;
    
    return json_syntax_error;
  }


  static bool has_next(term t) {
    return has_any_element_of(t, 
      syntax_element::next, syntax_element::wnext,
      syntax_element::prev, syntax_element::wprev
    );
  }

  uint8_t formula_features(formula f) {
    return f.match(
      [](boolean) -> uint8_t { return 0; },
      [](proposition) -> uint8_t { return 0; },
      [](atom a) -> uint8_t {
        uint8_t nextvar = 0;
        for(term t : a.terms())
          if(has_next(t))
            nextvar = (uint8_t)feature_t::nextvar;
        return nextvar | (uint8_t)feature_t::first_order;
      },
      [](equality, auto terms) -> uint8_t {
        for(auto t : terms)
          if(has_next(t))
            return 
              (uint8_t)feature_t::nextvar | (uint8_t)feature_t::first_order;

        return (uint8_t)feature_t::first_order;
      },
      [](comparison, term left, term right) -> uint8_t {
        if(has_next(left) || has_next(right))
          return (uint8_t)feature_t::nextvar | (uint8_t)feature_t::first_order;
        return (uint8_t)feature_t::first_order;
      },
      [](quantifier q) -> uint8_t { // LCOV_EXCL_LINE
        return (uint8_t)feature_t::first_order |
               (uint8_t)feature_t::quantifiers |
               formula_features(q.matrix());
      },
      [](tomorrow, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (formula_features(args) | ...);
      },
      [](w_tomorrow, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (formula_features(args) | ...);
      },
      [](yesterday, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (uint8_t)feature_t::past |
               (formula_features(args) | ...);
      },
      [](w_yesterday, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (uint8_t)feature_t::past |
               (formula_features(args) | ...);
      },
      [](always, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (formula_features(args) | ...);
      },
      [](eventually, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (formula_features(args) | ...);
      },
      [](once, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (uint8_t)feature_t::past |
               (formula_features(args) | ...);
      },
      [](historically, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (uint8_t)feature_t::past |
               (formula_features(args) | ...);
      },
      [](until, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (formula_features(args) | ...);
      },
      [](release, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (formula_features(args) | ...);
      },
      [](w_until, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (uint8_t)feature_t::past |
               (formula_features(args) | ...);
      },
      [](s_release, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (formula_features(args) | ...);
      },
      [](since, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (uint8_t)feature_t::past |
               (formula_features(args) | ...);
      },
      [](triggered, auto ...args) -> uint8_t {
        return (uint8_t)feature_t::temporal | (uint8_t)feature_t::past |
               (formula_features(args) | ...);
      },
      [](unary, formula arg) -> uint8_t { 
        return formula_features(arg);
      },
      [](disjunction d) {
        uint8_t features = 0;
        for(formula op : operands(d))
          features = features | formula_features(op);
        
        return features;
      },
      [](conjunction d) {
        uint8_t features = 0;
        for(formula op : operands(d))
          features = features | formula_features(op);
        
        return features;
      },
      [](binary, formula left, formula right) -> uint8_t {
        return formula_features(left) | formula_features(right);
      }
    );
  }
}
