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

#ifndef BLACK_PIPES_PIPES_HPP
#define BLACK_PIPES_PIPES_HPP

#include <black/ast/algorithms>

namespace black::pipes::internal {

  class id_t : public transform::base
  {
  public:
    id_t(class consumer *next) : _next{next} { }
      
    virtual class consumer *consumer() override { return _next; }

    virtual std::optional<logic::object> translate(logic::object) override { 
      return {};
    }

    virtual logic::term undo(logic::term t) override { return t; }

  private:
    class consumer *_next;
  };


  class composed_t : public transform::base
  {
  public:
    composed_t(
      class consumer *next, 
      transform::pipeline first, transform::pipeline second
    ) : _second{second(next)}, _first{first(_second->consumer())} { }
      
    virtual class consumer *consumer() override {
      return _first->consumer();
    }

    virtual std::optional<logic::object> translate(logic::object x) override { 
      auto f = _first->translate(x);
      if(!f)
        return {};
      return _second->translate(*f);
    }

    virtual logic::term undo(logic::term t) override {
      return _first->undo(_second->undo(t));
    }
  
  private:
    transform::instance _second;
    transform::instance _first;
  };

  class map_t : public transform::base
  {
  public:
    using type_mapping_t = std::function<logic::type(logic::type)>;
    using term_mapping_t = std::function<logic::term(logic::term)>;

    map_t(
      class consumer *next, 
      type_mapping_t ty_map, term_mapping_t te_map, term_mapping_t back_map
    );

    virtual ~map_t() override;
      
    virtual class consumer *consumer() override;

    virtual std::optional<logic::object> translate(logic::object x) override;

    virtual logic::term undo(logic::term x) override;

  private:
    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

}

namespace black::pipes {
  inline constexpr auto id = make_transform<internal::id_t>{};
  inline constexpr auto composed = make_transform<internal::composed_t>{};
  inline constexpr auto map = make_transform<internal::map_t>{};

  inline transform::pipeline 
  operator|(transform::pipeline first, transform::pipeline second) {
    return composed(std::move(first), std::move(second));
  }
}

#endif // BLACK_PIPES_PIPES_HPP
