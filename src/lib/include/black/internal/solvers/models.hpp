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

#ifndef BLACK_SOLVERS_MODELS_HPP
#define BLACK_SOLVERS_MODELS_HPP

#include <black/logic>

#include <limits>

namespace black::solvers {

  //!
  //! Class representing models.
  //!
  //! Models can come from different sources: solvers, files, runtime traces of
  //! monitored systems, etc. This class is a thin wrapper over a shared pointer
  //! to \ref model::base, which is the abstract base class for all these kinds
  //! of models.
  //!
  //! Independently from the source, all models can return a \ref term when
  //! given an \ref object and, for non-rigid entities, a time step.
  //!
  //! Models can represent temporal state sequences, or single a-temporal
  //! structures, depending on their source. Temporal models know their \ref
  //! size(), which can be finite or infinite.
  //!
  class model 
  {
  public:
    class base;
    using ptr = std::shared_ptr<const base>;
    
    //! A constant to represent the size of an infinite temporal model.
    static constexpr size_t infinite = std::numeric_limits<size_t>::max();

    //! \name Constructors
    //! @{

    //! Default constructor for an empty model.
    model();

    //! Constructor from a shared pointer to `model::base`
    model(ptr p);


    model(model const&) = default;
    model(model &&) = default;

    //! @}
    
    model &operator=(model const&) = default;
    model &operator=(model &&) = default;

    //! Returns a `term` associated to an `object`.
    //!
    //! \param x the `object` we want to extract the term of.
    //!
    //! Returns the term associated with `x` in the model, or an empty optional
    //! if this model does not know anything about `x` or if `x` does not refer
    //! to a rigid entity (in that case, one can use the other overload).
    std::optional<logic::term> value(logic::object x) const;

    //! Returns a `term` associated to an `object` at a given time stamp.
    //!
    //! \param x the object we want to extract the term of.
    //!
    //! \param t the timestamp.
    //!
    //! Returns the term associated with `x` in the model, or an empty optional
    //! if this model does not know anything about `x`.
    std::optional<logic::term> value(logic::object x, size_t t) const;

    //!
    //! Returns the size of the model.
    //!
    //! The returned value is:
    //! - zero, if the model is not temporal
    //! - a positive number if the model is temporal and finite
    //! - the `model::infinite` constant if the model is temporal and infinite
    //!
    size_t size() const;

  private:
    ptr _ptr;
  };

  class model::base 
  {
  public:
    base() = default;
    virtual ~base() = default;

    virtual std::optional<logic::term> value(logic::object x) const = 0;

    virtual std::optional<logic::term> value(logic::object x, size_t t) const = 0;

    virtual size_t size() const = 0;
  };

  class empty_t : public model::base 
  {
  public:
    empty_t() = default;
    virtual ~empty_t() override = default;
    
    virtual std::optional<logic::term> value(logic::object) const override { return {}; }

    virtual std::optional<logic::term> value(logic::object, size_t) const override { 
      return {}; 
    }

    virtual size_t size() const override { return 0; }
  };

  inline model::model() : _ptr{std::make_shared<empty_t>()} { }

  inline model::model(ptr p) : _ptr{p} { }

  inline std::optional<logic::term> model::value(logic::object x) const {
    return _ptr->value(x);
  }

  inline std::optional<logic::term> model::value(logic::object x, size_t t) const {
    return _ptr->value(x, t);
  }

  inline size_t model::size() const {
    return _ptr->size();
  }

}

#endif // BLACK_SOLVERS_MODELS_HPP
