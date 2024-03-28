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

#ifndef BLACK_PROCESSING_STUFF_HPP
#define BLACK_PROCESSING_STUFF_HPP

#include <black/support>
#include <black/logic>

#include <vector>
#include <array>

namespace black::processing {

  //!
  //! Holder value type for pipeline stages
  //! 
  class pipeline
  {
  public:
    class stage;
    class instance;

    template<typename S>
      requires std::is_base_of_v<stage, S>
    pipeline(S s) : _stage{std::move(s)} { }

    pipeline(pipeline const&) = default;
    pipeline(pipeline &&) = default;
    
    pipeline &operator=(pipeline const&) = default;
    pipeline &operator=(pipeline &&) = default;

    bool operator==(pipeline const&) const = default;

    pipeline::instance create(producer *prev) const;

    logic::module operator()(logic::module mod) const;

  private:
    support::any<stage> _stage;
  };

  //!
  //! Virtual base class for pipeline stages
  //!
  class pipeline::stage 
  {
  public:
    stage() = default;
    stage(stage const&) = default;
    stage(stage &&) = default;

    virtual ~stage() = default;
    
    stage &operator=(stage const&) = default;
    stage &operator=(stage &&) = default;

    bool operator==(stage const&) const = default;

    virtual pipeline::instance create(producer *prev) const = 0;
  };

  //!
  //! Move-only value type for pipeline instances
  //!
  class pipeline::instance 
  {
  public:
    instance(std::unique_ptr<class producer> inst) : _inst{std::move(inst)} { } 
    
    instance(instance const&) = delete;
    instance(instance &&) = default;
    
    instance &operator=(instance const&) = delete;
    instance &operator=(instance &&) = default;

    bool operator==(instance const&) const = default;
      
    class producer *producer() {
      return _inst.get();
    }

    class producer const *producer() const {
      return _inst.get();
    }

    logic::module operator()() {
      logic::module res;
      
      _inst->replay(_mod, &_mod);
      
      return _mod;
    }
  
  private:
    std::unique_ptr<class producer> _inst;
    logic::module _mod;
  };

  //
  // Out-of-line implementation of `pipeline` members
  //
  inline pipeline::instance pipeline::create(producer *prev) const {
    return _stage->create(prev);
  }

  inline logic::module pipeline::operator()(logic::module mod) const {
    instance inst = create(&mod);
    return inst();
  }

  //!
  //! Trivial stage that does nothing
  //!
  class id : public pipeline::stage
  {
  public:
    class instance;

    id() = default;
    id(id const&) = default;
    id(id &&) = default;
    
    id &operator=(id const&) = default;
    id &operator=(id &&) = default;

    bool operator==(id const&) const = default;

    virtual pipeline::instance create(producer *prev) const override;
  };
  
  //!
  //! Instance type for the id() pipeline stage
  //!
  class id::instance : public producer
  {
  public:
    instance(producer *prev) : _prev{std::move(prev)} { }
    virtual ~instance() override = default;

    virtual 
    void replay(logic::module base, consumer *next) override {
      _prev->replay(base, next);
    }

  private:
    producer *_prev;
  };

  inline pipeline::instance id::create(producer *prev) const {
    return {std::make_unique<instance>(std::move(prev))};
  }

  class composed : public pipeline::stage
  {
  public:
    class instance;

    composed(pipeline first, pipeline second)
      : _first{first}, _second{second} { }

    composed(composed const&) = default;
    composed(composed &&) = default;
    
    composed &operator=(composed const&) = default;
    composed &operator=(composed &&) = default;

    bool operator==(composed const&) const = default;

    virtual pipeline::instance create(producer *p) const override;

  private:
    pipeline _first;
    pipeline _second;
  };

  class composed::instance : public producer
  {
  public:
    instance(pipeline::instance first, pipeline::instance second) 
      : _first{std::move(first)}, _second{std::move(second)} { }

    virtual ~instance() override = default;

    virtual 
    void replay(logic::module base, consumer *next) override {
      _second.producer()->replay(base, next);
    }

  private:
    pipeline::instance _first;
    pipeline::instance _second;
  };

  inline pipeline::instance composed::create(producer *prev) const {
    auto f = _first.create(prev);
    auto s = _second.create(f.producer());
    
    return {std::make_unique<instance>(std::move(f), std::move(s))};
  }

  inline 
  pipeline operator|(pipeline first, pipeline second) {
    return composed(std::move(first), std::move(second));
  }

}

#endif // BLACK_PROCESSING_STUFF_HPP
