//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#include <black/sat/sat.hpp>
#include <black/sat/dimacs.hpp>

namespace black::sat::backends 
{
  class cmsat : public ::black::sat::dimacs::solver
  {
  public:
    cmsat();
    virtual ~cmsat();

    virtual void new_vars(size_t n);
    virtual size_t nvars() const;
    virtual void assert_clause(dimacs::clause f);
    virtual bool is_sat();
    virtual bool is_sat_with(std::vector<dimacs::literal> const& assumptions);
    virtual void clear();
    virtual std::optional<std::string> license() const;

  private:
    struct _cmsat_t;
    std::unique_ptr<_cmsat_t> _data;
  };
}
