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
#include <black/logic>
#include <black/ast/algorithms>
#include <black/pipes>
#include <iostream>

namespace black::pipes::internal {

  using namespace support;
  using namespace logic;
  using namespace ast;

  struct  debug_t::impl_t : public consumer
  {
    impl_t(class consumer *next) : _next{next}{ }

    class consumer *_next;

    std::optional<object> translate(object x);
    term undo(term x);

    virtual void import(logic::module) override;
    virtual void adopt(std::shared_ptr<logic::root const>) override;
    virtual void state(logic::term t, logic::statement s) override;
    virtual void push() override;
    virtual void pop(size_t) override;

    private:
    std::string term_to_string(term);

  };

  debug_t:: debug_t( class consumer *next ) : _impl{std::make_unique<impl_t>(next)} {
    std::cout << "Debug stage constructed." << std::endl;
  }

  debug_t::~ debug_t() = default;

  consumer * debug_t::consumer() { return _impl.get(); }

  std::optional<object>  debug_t::translate(object x) { 
    return _impl->translate(x); 
  }
  
  term  debug_t::undo(term x) { return _impl->undo(x); }

  void  debug_t::impl_t::import(logic::module m) {
    std::cout << "Debug -> import" << std::endl;
    _next->import(std::move(m));
  }

  void  debug_t::impl_t::adopt(std::shared_ptr<logic::root const>) {
    std::cout << "Debug -> adopt" << std::endl;
  }

  void  debug_t::impl_t::state(logic::term t, logic::statement s) {
    if(s == logic::statement::init) {
      std::cout << "Debug -> state -> init" << std::endl;
    }
    if(s == logic::statement::transition) {
      std::cout << "Debug -> state -> transition" << std::endl;
    }
    if(s == logic::statement::final) {
      std::cout << "Debug -> state -> final" << std::endl;
    }

    std::cout << term_to_string(t) << std::endl << std::endl;

  }

  void  debug_t::impl_t::push() {
    std::cout << "Debug -> push" << std::endl;
  }

  void  debug_t::impl_t::pop(size_t n) {
    std::cout << "Debug -> pop" << std::endl;
    _next->pop(n);
  }

  std::optional<object>  debug_t::impl_t::translate(object) { 
    std::cout << "Debug -> translate" << std::endl;
    return {};
  }

  term debug_t::impl_t::undo(term t) {
    std::cout << "Debug -> undo(" << term_to_string(t) << ")" << std::endl;
    return t;
  }

  std::string debug_t::impl_t::term_to_string(term t) {
    return match(t)(
      /*
        Object terms.
      */
      [](object obj)   {            
        if(obj.entity()->value.has_value()) { 
          return match(obj.entity()->value.value())(
            [&](integer i)  { return std::to_string(i.value()); },
            [&](real r)     { return std::to_string(r.value()); },                    
            [&](boolean b)  { return std::to_string(b.value()); },
            [obj] (lambda) { return obj.entity()->name.name().to_string(); }
          );
        }
        return obj.entity()->name.name().to_string();
      },
      [](variable var) { return var.name().to_string(); },

      /*
        Boolean and first-order predicates.
      */
      [&](equal, std::vector<term> arguments)      { return "(" + term_to_string(arguments[0]) + " == " + term_to_string(arguments[1]) + ")"; },
      [&](distinct, std::vector<term> arguments)   { return "(" + term_to_string(arguments[0]) + " != " + term_to_string(arguments[1]) + ")"; },
      
      [&](atom, term head, std::vector<term> arguments) {
        std::string result = term_to_string(head) + "(";
        for (auto i = arguments.begin(); i != arguments.end(); i ++) {
          if (i != arguments.begin()) result = result + ", ";
          result = result + term_to_string(*i);
        }
        return result + ")";
      }, 

      [&](exists, std::vector<decl> decls, term body) {
        std::string result = "exists ";
        for (decl d : decls) {
          result = result + term_to_string(d.name().head()) + " ";
        }
        result = result + ". (" + term_to_string(body) + ")";
        return result;
      },
      [&](forall, std::vector<decl> decls, term body) {
        std::string result = "forall ";
        for (decl d : decls) {
          result = result + term_to_string(d.name().head()) + " ";
        }
        result = result + ". (" + term_to_string(body) + ")";
        return result;
      },      

      /*
        Boolean connectives.
      */
      [&](negation, term argument) { return "!(" + term_to_string(argument) + ")"; },
      [&](conjunction, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " && " + term_to_string(arguments[1]) + ")"; },
      [&](disjunction, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " || " + term_to_string(arguments[1]) + ")"; },
      [&](implication, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " => " + term_to_string(arguments[1]) + ")"; },

      /*
        Future LTL operators.
      */
      [&](tomorrow, term argument)         { return "X("  + term_to_string(argument) + ")"; },
      [&](w_tomorrow, term argument)       { return "wX(" + term_to_string(argument) + ")"; },
      [&](eventually, term argument)       { return "F("  + term_to_string(argument) + ")"; },
      [&](always, term argument)           { return "G("  + term_to_string(argument) + ")"; },
      [&](until, term left, term right)    { return "("  + term_to_string(left) + " U " + term_to_string(right) + ")"; },
      [&](release, term left, term right)  { return "("  + term_to_string(left) + " R " + term_to_string(right) + ")"; },

      /*
        Past LTL operators.
      */
      [&](yesterday, term argument)            { return "Y(" + term_to_string(argument) + ")"; },
      [&](w_yesterday, term argument)          { return "Z(" + term_to_string(argument) + ")"; },
      [&](once, term argument)                 { return "O(" + term_to_string(argument) + ")"; },
      [&](historically, term argument)         { return "H(" + term_to_string(argument) + ")"; },
      [&](since, term left, term right)        { return "(" + term_to_string(left) + " S " + term_to_string(right) + ")"; },
      [&](triggered, term left, term right)    { return "(" + term_to_string(left) + " T " + term_to_string(right) + ")"; },

      /*
        Arithmetic operators.
      */
      [&](minus, term argument)                { return "-(" + term_to_string(argument) + ")"; },
      [&](sum, term left, term right)          { return "(" + term_to_string(left) + " + " + term_to_string(right) + ")"; },
      [&](product, term left, term right)      { return "(" + term_to_string(left) + " * " + term_to_string(right) + ")"; },
      [&](difference, term left, term right)   { return "(" + term_to_string(left) + " - " + term_to_string(right) + ")"; },
      [&](division, term left, term right)     { return "(" + term_to_string(left) + " / " + term_to_string(right) + ")"; },

      /*
        Relational comparisons.
      */
      [&](less_than, term left, term right)         { return "(" + term_to_string(left) + " < "  + term_to_string(right) + ")"; },
      [&](less_than_eq, term left, term right)      { return "(" + term_to_string(left) + " <= " + term_to_string(right) + ")"; },
      [&](greater_than, term left, term right)      { return "(" + term_to_string(left) + " > "  + term_to_string(right) + ")"; },
      [&](greater_than_eq, term left, term right)   { return "(" + term_to_string(left) + " >= " + term_to_string(right) + ")"; }
    );
  }
}
