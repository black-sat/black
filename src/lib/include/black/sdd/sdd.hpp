//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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
//

#ifndef BLACK_SDD_HPP
#define BLACK_SDD_HPP

#include <black/logic/logic.hpp>

struct sdd_node_t;
struct sdd_manager_t;
struct wmc_manager_t;

namespace black::sdd {

  class variable;
  class node;

  class manager {
  public:
    manager(alphabet *sigma);
    manager(manager const&) = delete;
    manager(manager &&);
    ~manager();
    
    manager &operator=(manager const&) = delete;
    manager &operator=(manager &&);

    class variable variable(proposition p);
    node top();
    node bottom();

  private:
    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

  class variable 
  {
  public:
    variable(variable const&) = default;
    variable(variable &&) = default;
    
    variable &operator=(variable const&) = default;
    variable &operator=(variable &&) = default;
    
    bool operator==(variable const&) const = default;

    proposition name() const { return _name; }
    class manager *manager() const { return _mgr; }

  private:
    friend class manager;

    variable(class manager *mgr, proposition name, unsigned var) 
      : _mgr{mgr}, _name{name}, _var{var} { }

    class manager *_mgr;
    proposition _name;
    unsigned _var;
  };

}

#endif // BLACK_SDD_HPP
