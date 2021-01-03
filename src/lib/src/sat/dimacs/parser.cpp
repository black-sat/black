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

#include <black/sat/dimacs/parser.hpp>

#include <cctype>
#include <fmt/format.h>

namespace black::sat::dimacs::internal 
{  
  struct parser::_parser_t {
    std::istream &in;
    std::function<void(std::string)> handler;
    bool start_of_line = true;

    _parser_t(std::istream &_in, std::function<void(std::string)> _handler)
       : in{_in}, handler{_handler} { }

    int get();
    void skip_comment();
    void skip();
    bool parse_header();
    std::optional<clause> parse_clause();
    std::vector<clause> parse_clauses();
  };

  parser::parser(std::istream &in, std::function<void(std::string)> handler) 
    : _data{std::make_unique<_parser_t>(in, handler)} { }
  parser::~parser() = default;

  int parser::_parser_t::get() {
    int c = in.get();
    start_of_line = (c == '\n');
    
    return c;
  }

  void parser::_parser_t::skip_comment() {
    if(in.peek() != 'c')
      return;
    
    while(in.good() && get() != '\n');
  }

  void parser::_parser_t::skip() {
    do {
      if(start_of_line)
        skip_comment();
      get();
    } while(in.good() && isspace(in.peek()));
  }

  bool parser::_parser_t::parse_header() {
    if(!in.good())
      return false;

    std::string header;
    std::getline(in, header);
    start_of_line = true;

    if(header.substr(0, 5) != "p cnf") {
      handler("expected problem header");
      return false;
    }

    return true;
  }

  std::vector<clause> parser::_parser_t::parse_clauses() {
    return {};
  }

  std::optional<problem> parser::parse() 
  {
    _data->skip();
    if(!_data->parse_header())
      return std::nullopt;

    _data->skip();
    return _data->parse_clauses();
  }
}
