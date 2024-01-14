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

#ifndef BLACK_IO_PRINTER_HPP
#define BLACK_IO_PRINTER_HPP

#include <black/logic>

#include <format>

namespace black::io {
  
  struct syntax {
    struct python { };
    // struct black { }; // TODO...
    // struct smtlib2 { };

    syntax(python s) : settings{s} { }

    std::variant<python /*, black, smtlib2 */> settings;
  };

  std::string format(syntax::python, logic::term v);
  
  inline std::string format(syntax s, logic::term v) {
    return support::match(s.settings)(
      [&](auto x) { return format(x, v); }
    );
  }

}

#endif // BLACK_IO_PRINTER_HPP
