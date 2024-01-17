//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <black/support>
#include <black/ast/core>
#include <black/io>

namespace black::io {

  static std::string join() { return ""; }
  static std::string join(std::string s, auto ...others) {
    ((s += ", " + others), ...);
    return s;
  }

  static std::string join(std::vector<std::string> strings) {
    if(strings.empty())
      return "";
    std::string s = strings[0];
    for(size_t i = 1; i < strings.size(); i++)
      s += ", " + strings[i];
    return s;
  }
  
  template<typename T>
  static std::string format(syntax::python s, std::vector<T> vec) {
    std::vector<std::string> strings;
    for(auto v : vec)
      strings.push_back(format(s, v));
    return std::format("[{}]", join(strings));
  }

  static std::string format(syntax::python s, logic::decl d) {
    return std::format("Decl({}, {})", format(s, d.name), format(s, d.type));
  }
  
  static std::string format(syntax::python s, logic::pattern p) {
    return std::format("Pattern({}, {})", format(s, p.head), format(s, p.body));
  }
  
  static std::string format(syntax::python, ast::core::label label) {
    if(auto s = label.get<std::string>(); s)
      return std::format("\"{}\"", *s);
    return label.to_string();
  }
  
  template<typename T>
    requires std::is_fundamental_v<T>
  static std::string format(syntax::python, T v) {
    return std::format("{}", v);
  }

  std::string format(syntax::python s, logic::term v) {
    using namespace support;
    using namespace ast::core;
    using namespace logic;

    return match(v)(
      [&]<typename Node>(Node, auto ...fields) {
        return std::format(
          "{}({})",
          to_camel(ast_node_name_v<term, Node>), 
          join(format(s, fields)...)
        );
      }
    );
  }

}