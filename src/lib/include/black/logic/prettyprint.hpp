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

#ifndef BLACK_LOGIC_PRETTY_PRINT_HPP
#define BLACK_LOGIC_PRETTY_PRINT_HPP

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>

#include <string>

namespace black_internal::logic
{
  
  BLACK_EXPORT
  std::string to_string(formula<LTLPFO> f);
  
  BLACK_EXPORT
  std::string to_string(term<LTLPFO> t);
  
  BLACK_EXPORT
  std::string to_string(relation t);
  
  BLACK_EXPORT
  std::string to_string(function t);

}

#endif // BLACK_LOGIC_PRETTY_PRINT_HPP

