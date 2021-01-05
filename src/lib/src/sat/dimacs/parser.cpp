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

#include <black/sat/dimacs.hpp>
#include <black/support/config.hpp>

#include <cctype>
#include <cmath>
#include <fmt/format.h>

namespace black::sat::dimacs::internal 
{  
  struct _parser_t {
    std::istream &in;
    std::function<void(std::string)> handler;

    _parser_t(std::istream &_in, std::function<void(std::string)> _handler)
       : in{_in}, handler{_handler} { }

    void skip_comment();
    void skip();
    bool parse_header();
    std::optional<literal> parse_literal();
    std::vector<clause> parse_clauses();
    std::optional<problem> parse();
  };

  void _parser_t::skip_comment() {
    if(in.peek() != 'c')
      return;
    
    while(in.good() && in.peek() != '\n')
      in.get();
  }

  void _parser_t::skip() {
    while(in.good() && (isspace(in.peek()) || in.peek() == 'c')) {
      skip_comment();
      in.get();
    }
  }

  bool _parser_t::parse_header() {
    if(!in.good())
      return false;

    std::string h;
    for(int i = 0; i < 5; ++i)
      h += (char)in.get();
    
    if(h != "p cnf") {
      handler("expected problem header");
      return false;
    }

    uint32_t nvars = 0;
    uint64_t nclauses = 0;

    in >> nvars; // we ignore nvars and nclauses, but they must be there
    in >> nclauses;
    
    if(in.fail()) {
      handler("expected nbvars and nbclauses in problem header");
      return false;
    }

    return true;
  }

  std::optional<literal> _parser_t::parse_literal() 
  {
    if(in.eof())
      return std::nullopt;
      
    int32_t v = 0;
    if(!(in >> v)) {
      handler("expected literal");
      return std::nullopt;
    }

    return literal{ 
      /*sign=*/ (v >= 0), 
      /*var=*/  static_cast<uint32_t>(abs(v))
    };
  }

  std::vector<clause> _parser_t::parse_clauses() 
  {
    std::vector<clause> clauses;

    clause cl;
    do
    {
      skip();
      std::optional<literal> l = parse_literal();
      if(!l) {
        if(cl.literals.size() > 0)
          handler("expected '0' at the end of clause");
        return clauses;
      }
      if(l->var == 0) {
        clauses.push_back(cl);
        cl.literals.clear();
      } else {
        cl.literals.push_back(*l);
      }
    } while (!in.eof());
    
    return clauses;
  }

  std::optional<problem> _parser_t::parse() 
  {
    skip();
    if(!parse_header())
      return std::nullopt;

    skip();
    return problem{parse_clauses()};
  }

  std::optional<problem> parse(
    std::istream &in, std::function<void(std::string)> handler
  ) {
    _parser_t parser{in, handler};

    return parser.parse();
  }

  std::string to_string(literal l) {
    return fmt::format("{}{}", l.sign ? "" : "-", l.var);
  }

  void print(std::ostream &out, problem p) {
    out << fmt::format("c BLACK v{}\n", black::version);

    size_t nclauses = p.clauses.size();
    uint32_t nvars = 0;
    for(dimacs::clause c : p.clauses)
      for(dimacs::literal l : c.literals)
        nvars = l.var > nvars ? l.var : nvars;

    out << fmt::format("p cnf {} {}\n", nvars, nclauses);

    for(dimacs::clause c : p.clauses) {
      for(dimacs::literal l : c.literals) {
        out << to_string(l) << ' ';
      }
      out << "0\n";
    }
  }

  void print(std::ostream &out, std::optional<solution> const& s) {
    out << fmt::format("c BLACK v{}\n", black::version);

    if(!s) {
      out << "s UNSATISFIABLE\n";
      return;
    }

    out << "s SATISFIABLE\n";

    out << "v ";
    int i = 0;
    for(dimacs::literal l : s->assignments) {
      if(i % 4)
        out << "\nv ";
      out << to_string(l) << ' ';
    }
    out << '\n';
  }
}
