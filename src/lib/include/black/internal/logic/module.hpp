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

#ifndef BLACK_LOGIC_SCOPE_HPP
#define BLACK_LOGIC_SCOPE_HPP

#include <optional>
#include <expected>
#include <variant>
#include <memory>

#include <black/support>

namespace black::logic {

  using ast::core::label;

  struct lookup;

  //! The resolution behavior of declarations and definitions in \ref module.
  //!
  //! Used in functions module::declare() and module::define() to specify
  //! whether a declared/defined entity must be resolved immediately or at the
  //! next call to module::resolve().
  enum class resolution : bool {
    delayed = false, //!< wait the next call to module::resolve().
    immediate = true //!< resolve immediately.
  };

  //!
  //! The specification of an entity to be defined in a module.
  //!
  struct def {
    variable name; //!< the name of the entity.
    term type; //!< the type of the entity.
    term value; //!< the defining value of the entity.

    //! \name Constructors
    //!@{
    
    //! Constructs the \ref def object.
    def(variable name, term type, term value) 
      : name{name.name()}, type{type}, value{value} { }
    
    //! Constructs the \ref def object with the type set to `inferred_type()`,
    //! to be inferred automatically from `value`.
    def(variable name, term value)
      : def{name, inferred_type(), value} { }

    //!@}
  };

  //!
  //! The specification of a function to be defined in a module.
  //!
  struct function_def {
    variable name; //!< the name of the function.
    std::vector<decl> parameters; //!< the parameters of the function.
    term range; //!< the range (return type) of the function.
    term body; //!< the body of the function.

    //! \name Constructors
    //!@{

    //! Constructs a \ref function_def
    function_def(variable name, std::vector<decl> parms, term range, term body)
      : name{name}, parameters{std::move(parms)}, range{range}, body{body} { }

    //! Constructs a \ref function_def where the type is set to
    //! `inferred_type()`, to be inferred automatically from the function body.
    function_def(variable name, std::vector<decl> parms, term body)
      : name{name}, 
        parameters{std::move(parms)}, 
        range{inferred_type()}, 
        body{body} { }

    //!@}
  };

  class module;

  //!
  //! The scope resolution mode for `module::resolve()` and `module::adopt()`.
  //!
  enum class scope {
    recursive, //!< Recursive name resolution
    linear //!< Non-recursive name resolution
  };

  template<typename T>
  concept replay_target = requires(T v, object obj, module m, term t) {
    v.import(m);
    v.adopt(std::vector<object>{}, scope::recursive);
    v.require(t);
    v.push();
    v.pop(42);
  };

  //!
  //! Entry point to build modules.
  //!
  //! \ref module is a **regular** type (also known as *value type*), meaning
  //! that its instances are meant to be freely copied and passed around by
  //! value. Copy of a \ref module is cheap thanks to the underlying usage of
  //! persistent data structures.
  //!
  //! The \ref module class mantains sets of declarations, requirements
  //! (sometimes called *assertions*), and other elements in a stack that can be
  //! manipulated with the push() and pop() member functions.
  //!
  class module
  {
  public:

    //! \name Constructors, assignment and comparison
    //!@{

    //! Default constructor. Constructs an empty module.
    module();

    //! Copy constructor.
    module(module const&);

    //! Move constructor.
    module(module &&);
    
    ~module();
    
    //! Copy-assignment operator
    module &operator=(module const&); 

    //! Move-assignment operator
    module &operator=(module &&); 

    //! Equality comparison operator
    bool operator==(module const&) const;

    //!@}
    
    //! \name Declarations, definitions, and name lookup.
    //!@{
    
    //!
    //! Imports another module.
    //!
    //! \param m the imported module.
    //!
    //! The names visible in the imported modules will be visible in this one,
    //! but will be **shadowed** by those with the same name declared in this
    //! module.
    //!
    //! If two imported modules declare the same names, the module imported last
    //! has precedence and shadows the others.
    //!
    void import(module m);
  
    //!
    //! Declares a new entity.
    //!
    //! \param d the \ref decl object describing the new declaration. 
    //!
    //! \param r the resolution mode (see \ref resolution). 
    //!
    //! \returns an \ref object representing the newly declared entity.
    //!
    //! Declares an entity of name `d.name` and type `d.type`. 
    //!
    //! The resolution mode `r` determines when name lookup is applied to
    //! unbound variables:
    //! - if `r` is \ref resolution::immediate, any unbound \ref variable
    //!   present in `d.type` is resolved immediately using \ref resolved();
    //! - if `r` is \ref resolution::delayed, the name will **not** be visible
    //!   until the next call to the resolve() function, which will resolve
    //!   `d.type`.
    //!
    object declare(decl d, resolution r = resolution::immediate);

    //!
    //! Declares a new entity.
    //!
    //! \param name the name of the entity.
    //!
    //! \param type the type of the entity.
    //!
    //! \param r the resolution mode (see \ref resolution)
    //!
    //! Calls `declare(decl{name, type}, r)`.
    //!
    object 
    declare(variable name, term type, resolution r = resolution::immediate);
    
    //!
    //! Defines a new entity.
    //!
    //! \param d the \ref def object describing the new definition.
    //!
    //! \param r the resolution mode (see \ref resolution).
    //!
    //! \returns an \ref object representing the newly defined entity.
    //!
    //! Defines an entity of name `d.name`, type `d.type` and value `d.value`.
    //! If `d.type` is an \ref inferred_type term, the type is inferred from
    //! `d.value`, when possible (for now, recursive definitions cannot be
    //! type-inferred).
    //! 
    //! The resolution mode `r` determines when name lookup is applied to
    //! unbound variables:
    //! - if `r` is \ref resolution::immediate, any unbound \ref variable
    //!   present in `d.type` is resolved immediately using \ref resolved();
    //! - if `r` is \ref resolution::delayed, the name will **not** be visible
    //!   until the next call to the resolve() function, which will resolve
    //!   `d.type`.
    //!
    object define(def d, resolution r = resolution::immediate);

    //!
    //! Defines a new entity.
    //!
    //! \param name the name of the entity.
    //!
    //! \param type the type of the entity.
    //!
    //! \param value the defining value of the entity.
    //!
    //! \param r the resolution mode (see \ref resolution)
    //!
    //! Calls `define(def{name, type, value}, r)`.
    //!
    object define(
      variable name, term type, term value, resolution r = resolution::immediate
    );

    //!
    //! Defines a new entity with an inferred type.
    //!
    //! \param name the name of the entity.
    //!
    //! \param value the defining value of the entity.
    //!
    //! \param r the resolution mode (see \ref resolution)
    //!
    //! Calls `define(def{name, value}, r)`.
    //!
    object define(
      variable name, term value, resolution r = resolution::immediate
    );

    //!
    //! Defines a new function.
    //!
    //! \param f the \ref function_def object describing the new function.
    //!
    //! \param r the resolution mode (see \ref resolution).
    //!
    //! \returns an \ref object representing the newly defined function.
    //!
    //! Defines a function of name `f.name`, parameters described by \ref decl
    //! objects in `f.parameters`, range `f.range`, and body `f.body`. If
    //! `f.type` is an \ref inferred_type term, then the *range* type is
    //! inferred from `f.body`, when possible (for now, recursive definitions
    //! cannot be type-inferred).
    //!
    //! This function is equivalent to calling `define()` with a `def` object
    //! specifying a `function_type` type and a  `lambda` with matching
    //! parameters as value.
    //!
    //! The resolution mode `r` determines when name lookup is applied to
    //! unbound variables:
    //! - if `r` is \ref resolution::immediate, any unbound \ref variable
    //!   present in `d.type` is resolved immediately using \ref resolved();
    //! - if `r` is \ref resolution::delayed, the name will **not** be visible
    //!   until the next call to the resolve() function, which will resolve
    //!   `f.type`.
    //!
    object define(function_def f, resolution r = resolution::immediate);

    //!
    //! Defines a new function.
    //!
    //! \param name the name of the function.
    //!
    //! \param parameters the parameters of the function.
    //!
    //! \param range the range (return type) of the function.
    //!
    //! \param body the body of the function.
    //!
    //! \param r the resolution mode (see \ref resolution)
    //!
    //! Calls `define(function_def{name, parameters, range, body}, r)`.
    //!
    object define(
      variable name, std::vector<decl> parameters, term range, term body,
      resolution r = resolution::immediate
    );
    
    //!
    //! Defines a new function with an inferred return type.
    //!
    //! \param name the name of the function.
    //!
    //! \param parameters the parameters of the function.
    //!
    //! \param body the body of the function.
    //!
    //! \param r the resolution mode (see \ref resolution)
    //!
    //! Calls `define(function_def{name, parameters, body}, r)`.
    //!
    object define(
      variable name, std::vector<decl> parameters, term body,
      resolution r = resolution::immediate
    );

    //!
    //! Adopts a set of existing possibly recursive objects (declared or
    //! defined).
    //!
    //! \param objs the vector of objects to adopt.
    //!
    //! \param s the scope resolution mode (see \ref scope)
    //!
    //! Adopts in this module a set of external objects, possibly created using
    //! other modules. The entities referred to by the objects become visible in
    //! the module as if they were declared or defined by the declare() or
    //! define() functions.
    //!
    //! The scope resolution mode determines whether the adopted objects can
    //! potentially recursively refer to other objects of the same set
    //! (themselves included).
    //!
    void adopt(std::vector<object> const& objs, scope s = scope::recursive);

    //!
    //! Looks up a name in the current module and returns the corresponding
    //! object.
    //!
    //! \param x the \ref variable to look up.
    //!
    //! \returns the \ref object associated to `x` in the current module, or an
    //! empty optional if the name cannot be found.
    //!
    //! Name lookup is performed in this order:
    //! 1. the entities declared, defined, or adopted in the current module, in
    //!    the reverse order (i.e. most recent entities shadow previous ones);
    //! 2. the entities declared, defined, or adopted in any imported module,
    //!    with more recently imported modules shadowing previous ones.
    //!
    std::optional<object> lookup(variable x) const;

    //!
    //! Resolves unbound variables in a term by name lookup in the current
    //! module.
    //!
    //! \param t a term.
    //!
    //! \returns the resolved term
    //!
    //! Returns a term obtained from the given term `t` where each unbound
    //! variable is replaced by the corresponding object as found by lookup(),
    //! or is left unchanged if lookup() returns an empty optional.
    //!
    term resolved(term t) const;

    //!
    //! Resolves and completes the declaration/definition of pending entities.
    //!
    //! \param s the scope resolution mode (see \ref scope).
    //!
    //! This function completes the declaration or definition of entities
    //! declared or defined with declare() or define() where the `r` parameter
    //! was set to resolution::delayed. The terms employed for types and bodies
    //! of the entities are resolved similarly to resolved() using name lookup
    //! in the current module.
    //!
    //! The scope resolution mode determines whether recursion is allowed.
    //! - if `s` is scope::linear, the pending entities **do not** see each
    //!   other, and therefore cannot refer recursively to each other or
    //!   themselves.
    //! - if `s` is scope::recursive, all the pending entities, regardless of
    //!   the order of declaration/definiton, see each other, and thus can refer
    //!   recursively to any other entity in the group (themselves included).
    //!
    //! \note As also noted in define(), types of recursive definitions cannot
    //! be automatically inferred, at the moment.
    void resolve(scope s = scope::linear);

    //!@}

    //! \name Specifications and requirements
    //!@{

    //!
    //! Adds a requirement to the module.
    //!
    //! \param req the required term.
    //!
    //! Asserts that `req` must hold in any model of the module.
    //!
    void require(term req);

    //!@}

    //! \name Stack management
    //!@{

    //!
    //! Pushes a new frame on the module's stack.
    //!
    void push();

    //!
    //! Pops a given number of frames from the module's stack.
    //!
    //! \param n the number of frames to pop.
    //!
    //! Calls to this function cause any call to the import(), declare(),
    //! define(), adopt(), and require() functions, executed after the last `n`
    //! calls to push(), to be undone.
    //!
    //! If less than `n` calls to push() have been issued before (e.g. `n==1`
    //! and no call to push() happened at all), the module is reset empty, as
    //! just after default construction.
    //!
    //! \note Declarations/definitions that are pending because of calls to
    //! declare() or define() with delayed resolution mode are not affected by
    //! push() and pop() calls until they are resolved with resolve().
    //!
    void pop(size_t n = 1);

    //!@}

    //! \name Module traversal
    //!@{

    //!
    //! Replays all the differences of the current module from a given one.
    //!
    //! \param from the module starting point of the replay.
    //!
    //! \param target the object where replay calls are issued.
    //!
    //! Issues a sequence of calls to a set of member functions of `target` for
    //! each difference between `*this` and `from`.
    //! 1. a call to `target.import(m)` for each module `m` imported in `*this`
    //!    but not in `from`.
    //! 2. a call to `target.adopt(objs, s)`, for each set of entities
    //!    declared/defined/adopted in `*this` but not in `from`.
    //! 3. a call to `target.require(req)` for each term `req` required in
    //!    `*this` but not in `from`.
    //! 4. a call to `target.push()` for each frame level pushed in `*this` but
    //!    not in `from` (in the suitable order w.r.t. the previous items).
    //! 5. a call to `target.pop(n)` for each `n` frames that have to be popped
    //!    from `from` to make it a subset of `*this`.
    //!
    //! The types of the arguments of each call are the same as the
    //! corresponding member functions of \ref module with the same name.
    //!
    //! The calls are arranged in such a way that, *for any* pair of modules `m`
    //! and `from`, if `m` has no pending declaration/definitions, and the
    //! following call is issued:
    //!
    //! ```
    //!    m.replay(from, from);
    //! ```
    //!
    //! then `m == from` is guaranteed to hold.
    //!
    //! Note that a copy construction is sufficient to achieve the same. The
    //! purpose of this function is rather to be used with `target` objects
    //! of different types, as a way of observing the changes made to a module
    //! w.r.t. to `from`.
    //!
    //! This can also be used as a general iteration mechanism over all the
    //! constituent elements of the module (e.g. by providing an empty `from`
    //! module).
    //!
    //! \note Declarations/definitions that are pending because of calls to
    //! declare() or define() with delayed resolution mode are not replayed by
    //! `replay`, until they are resolved with resolve().
    template<replay_target T>
    void replay(module const& from, T &target) const;

    //!@}

  private:

    struct replay_target_t {
      virtual ~replay_target_t() = default;
      virtual void import(module) = 0;
      virtual void adopt(std::vector<object> const&, scope s) = 0;
      virtual void require(term) = 0;
      virtual void push() = 0;
      virtual void pop(size_t) = 0;
    };

    void replay(module const& from, replay_target_t *target) const;

    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

  //!
  //! Internal representation of an entity declared or defined in a module.
  //!
  //! A \ref lookup object represents an entity declared or defined in a module.
  //! Lookup objects are stored internally in modules and are usually not
  //! handled explicitly during normal usage of the API.
  //!
  //! \ref lookup inherits publicly from `std::enable_shared_from_this`, and
  //! \ref lookup objects are always handled internally by \ref module by means
  //! of `std::shared_ptr`, therefore a call to the `shared_from_this()` member
  //! function can be used to recover a strong reference to the object.
  //! Nevertheless, handling lookups by constructing a wrapping \ref object
  //! instance is preferred.
  //!
  struct lookup : std::enable_shared_from_this<lookup> {
    variable name; //!< The name of the entity
    term type; //!< The type of the entity
    std::optional<term> value; //!< The value of the entity. 
                               //!< Empty if the entity is declared

    //! \name Constructors
    //!@{

    //! Constructs the lookup from a \ref decl.
    explicit lookup(decl d) : name{d.name}, type{d.type} { }
    
    //! Constructs the lookup from a \ref def
    explicit lookup(def d) : name{d.name}, type{d.type}, value{d.value} { }

    //@}
  };

  //
  // Implementation of the module::replay interface
  //
  template<replay_target T>
  void module::replay(module const &from, T &target) const {
    struct wrap_t : replay_target_t {
      T *t;

      wrap_t(T *t) : t{t} { }

      virtual void import(module m) override { return t->import(std::move(m)); }
      virtual void adopt(std::vector<object> const& objs, scope s) override { 
        return t->adopt(objs, s); 
      }
      virtual void require(term r) override { return t->require(r); }
      virtual void push() override { t->push(); }
      virtual void pop(size_t n) override { t->pop(n); }

    } wrap{&target};

    return replay(from, &wrap);
  }

}

#endif // BLACK_LOGIC_SCOPE_HPP
