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
  class module;
  struct root;

  enum class statement : uint8_t;
}

namespace black::pipes {

  //!
  //! Abstract base class for objects capable of receiving streams of elements
  //! from modules and pipeline stages.
  //!
  //! Consumers are the objects that can be passed to the module::replay()
  //! function and receive a stream of elements (declarations, statements, etc.)
  //! from a module or from a preceding pass of a processing pipeline.
  //!
  //! Modules are consumer themselves.
  //!
  class consumer 
  {
  public:
    consumer() = default;
    consumer(consumer const&) = default;
    consumer(consumer &&) = default;

    virtual ~consumer() = default;

    consumer &operator=(consumer const&) = default;
    consumer &operator=(consumer &&) = default;

    bool operator==(consumer const&) const = default;

    //! Streams the import of a module.
    virtual void import(logic::module) = 0;

    //! Streams the adoption of a \ref root object managing a set of
    //! declared/defined \ref entity objects. In calls to adopt(), `r` cannot be
    //! null.
    virtual void adopt(std::shared_ptr<logic::root const> r) = 0;

    //! Streams the statement of a requirement or an automaton constraint.
    virtual void state(logic::term, logic::statement s) = 0;

    //! Streams the push of a frame on the assertion stack.
    virtual void push() = 0;

    //! Streams the pop of `n` frames from the assertion stack. Note that `n`
    //! might be greater than the number of previous push() calls.
    virtual void pop(size_t n) = 0;
  };

}

namespace black::logic {

  using ast::core::label;

  class module;
  struct entity;
  struct root;

  //! The resolution behavior of declarations and definitions in \ref module.
  //!
  //! Used in functions module::declare() and module::define() to specify
  //! whether a declared/defined entity must be resolved immediately or at the
  //! next call to module::resolve().
  enum class resolution : bool {
    delayed = false, //!< wait the next call to module::resolve().
    immediate = true //!< resolve immediately.
  };

  //
  // The following are here only to be extracted by Doxygen.
  // The real declarations of `decl` and `def` are in terms-defs.hpp
  //
#ifdef DOXYGEN_IS_RUNNING

  //!
  //! Which role a declaration have in a module.
  //!
  enum class role : uint8_t {
    rigid, //!< rigid declaration, does not change over time
    input, //!< input variable of the module
    state, //!< state variable of the module
    output //!< output variable of the module
  };

  //!
  //! The specification of an entity to be declared in a module.
  //!
  struct decl { 
    logic::variable name; //!< The name of the entity.
    logic::types::type type; //!< The type of the entity.
    
    //! The role of the entity.
    //! The role is ignored when \ref decl is used in \ref lambda terms.
    enum role role; 

    //! \name Constructors
    //!@{

    //! Constructs the object.
    inline decl(variable name, logic::types::type type, enum role role);

    //! Constructs the object with a default role of `role::rigid`.
    inline decl(logic::variable name, logic::types::type type);

    //!@}

    //! Equality comparison.
    bool operator==(decl const&) const = default;

    //! Hash function.
    inline size_t hash() const;
  };

  //!
  //! The specification of an entity to be defined in a module.
  //!
  struct def {
    variable name; //!< the name of the entity.
    types::type type; //!< the type of the entity.
    term value; //!< the defining value of the entity.

    //! \name Constructors
    //!@{
    
    //! Constructs the \ref def object.
    def(variable name, types::type type, term value);
    
    //! Constructs the \ref def object with the type set to `inferred_type()`,
    //! to be inferred automatically from `value`.
    def(variable name, type value);

    //!@}
  };

#endif

  //!
  //! The specification of a function to be defined in a module.
  //!
  struct function_def {
    variable name; //!< the name of the function.
    std::vector<decl> parameters; //!< the parameters of the function.
    type range; //!< the range (return type) of the function.
    term body; //!< the body of the function.

    //! \name Constructors
    //!@{

    //! Constructs a \ref function_def
    function_def(variable name, std::vector<decl> parms, type range, term body)
      : name{name}, parameters{std::move(parms)}, range{range}, body{body} { }

    //! Constructs a \ref function_def where the type is set to
    //! `inferred_type()`, to be inferred automatically from the function body.
    function_def(variable name, std::vector<decl> parms, term body)
      : name{name}, 
        parameters{std::move(parms)}, 
        range{types::inferred()}, 
        body{body} { }

    //!@}
  };

  //!
  //! The scope resolution mode for `module::resolve()` and `root::mode`
  //!
  enum class recursion : bool {
    forbidden = false, //!< Non-recursive name resolution
    allowed = true     //!< Recursive name resolution
  };

  //!
  //! Kind of statements that can be given to `module::state()`.
  //!
  enum class statement : uint8_t {
    requirement, //!< A requirement
    init, //!< A constraint on the initial state
    transition, //!< A constraint on the transition relation
    final //!< A constraint on the final state
  };

  //!
  //! Entry point to build modules.
  //!
  //! The \ref module class mantains sets of declarations, requirements
  //! (sometimes called *assertions*), and other elements in a stack that can be
  //! manipulated with the push() and pop() member functions.
  //!
  //! \ref module is a **regular** type (also known as *value type*), *i.e.*, it
  //! is *default-initializable*, *copy-constructible*, *move-constructible*,
  //! and *equality-comparable*. Thanks to the underlying usage of persistent
  //! data structures, all these operations are also quite fast, so \ref module
  //! objects are meant to be freely passed around by value. A dynamic
  //! allocation of a small object is still needed for copies, so moving is
  //! still preferred when possible.
  //!
  //! \note Declarations/definitions that are pending because of calls to
  //! declare() or define() with delayed resolution mode are **not** retained in
  //! copies.
  //!
  //! Equality is checked *deeply* against the *contents* of the modules.
  //!
  //! Thanks to persistent data structures, equality comparison is relatively
  //! cheap, but some attention has to be payed. Because of how the underlying
  //! data structures work, it is cheap to compare two copies of the same
  //! module, or two modules where one has been derived from a copy of the other
  //! by a few edits.
  //!
  //! For example, the following is very cheap:
  //!
  //! ```cpp
  //! module m = very_large_module();
  //! module m2 = m; // cheap copy
  //!
  //! assert(m2 == m); // the cheapest
  //!
  //! m2.require(x == y);
  //!
  //! assert(m != m2); // still cheap
  //! ```
  //!
  //! Comparing two different modules is also usually cheap because of
  //! short-curcuiting. However, comparing completely unrelated modules that
  //! somehow happen to be equal or very similar is potentially expensive,
  //! because all the contents of the modules will have to be compared.
  //!
  class module : public pipes::consumer
  {
  public:

    //! \name Constructors, assignment and comparison
    //!@{

    //! Default constructor. Constructs an empty module.
    module();

    module(module const&);

    module(module &&);
    
    virtual ~module() override;
    
    //! Copy-assignment operator
    module &operator=(module const&);

    //! Move-assignment operator
    module &operator=(module &&); 

    //! Equality comparison operator.
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
    virtual void import(module m) override;
  
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
    //! \note Declarations that are pending because of calls to declare() or
    //! with delayed resolution mode are **not** retained in copies.
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
    declare(
      variable name, types::type type, 
      enum role role = role::rigid, 
      resolution r = resolution::immediate
    );
    
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
    //! \note Definitions that are pending because of calls to define() with
    //! delayed resolution mode are **not** retained in copies.
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
      variable name, types::type type, term value, 
      resolution r = resolution::immediate
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
      variable name, std::vector<decl> parameters, types::type range, term body,
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
    //! Adopts a \ref root and all the entities it collects.
    //!
    //! \param r a shared pointer to the \ref root to be adopted
    //!
    //! Adopts in this module a set of entities, possibly created using other
    //! modules, collected by the given \ref root. The entities referred to by
    //! the objects become visible in the module as if they were declared or
    //! defined by the declare() or define() functions.
    //!
    //! If the pointer is `nullptr` the function does nothing.
    //!
    virtual void adopt(std::shared_ptr<root const> r) override;

    //!
    //! Adopts an object and its siblings.
    //!
    //! \param obj the object to adopt.
    //!
    //! The entity represented by the given object is adopted together with all
    //! the entities collected by the same root.
    //!
    //! The call to `m.adopt(obj)` is roughly the same as
    //! `m.adopt(obj.entity()->root.lock())`.
    //!
    //! If `obj.entity()->root` is null, the function does nothing. This happens
    //! when the object has been returned by a call to `declare()` or `define()`
    //! with `resolution::delayed` and `resolve()` has not been called yet.
    //!
    void adopt(object obj);

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
    //! \returns a shared pointer to the \ref root collecting the entities
    //! resolved in this call.
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
    //! The
    //! \note As also noted in define(), types of recursive definitions cannot
    //! be automatically inferred, at the moment.
    std::shared_ptr<root const> resolve(recursion s = recursion::forbidden);

    //!@}

    //! \name Specifications and requirements
    //!@{

    //!
    //! Adds a statement to the module.
    //!
    //! \param t the statement.
    //!
    //! \param s the kind of statement
    //!
    //! Statements can be of different kinds:
    //! 1. requirements (`statement::requirement`):
    //!    asserts that `t` holds in any model of the module;
    //! 2. initial states (`statement::init`):
    //!    asserts that `t` holds in the initial state of the module;
    //! 3. transitions (`statement::transition`):
    //!    asserts that `t` holds during any transition of the module;
    //! 4. final states (`statement::final`):
    //!    asserts that `req` must hold at the end of any execution of ht e
    //!    module.
    //!
    virtual void state(logic::term t, statement s) override;

    //!
    //! Same as `state(req, statement::requirement)`.
    //!
    void require(term req) { state(req, statement::requirement); }
    
    //!
    //! Same as `state(req, statement::init)`.
    //!
    void init(term t) { state(t, statement::init); }
    
    //!
    //! Same as `state(t, statement::transition)`.
    //!
    void transition(term t) { state(t, statement::transition); }
    
    //!
    //! Same as `state(t, statement::final)`.
    //!
    void final(term t) { state(t, statement::final); }

    //!@}

    //! \name Stack management
    //!@{

    //!
    //! Pushes a new frame on the module's stack.
    //!
    virtual void push() override;

    //!
    //! Pops a given number of frames from the module's stack.
    //!
    //! \param n the number of frames to pop.
    //!
    //! Calls to this function cause any call to the import(), declare(),
    //! define(), adopt(), and state() functions, executed after the last `n`
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
    virtual void pop(size_t n = 1) override;

    //!@}

    //! \name Module traversal
    //!@{

    //!
    //! Replays all the differences of the current module from a given one.
    //!
    //! \param from the module starting point of the replay.
    //!
    //! \param target a pointer to the object where replay calls are issued.
    //!
    //! Issues a sequence of calls to a set of member functions of `target` for
    //! each difference between `*this` and `*from`.
    //! 1. a call to `target.import(m)` for each module `m` imported in `*this`
    //!    but not in `from`.
    //! 2. a call to `target.adopt(objs, s)`, for each set of entities
    //!    declared/defined/adopted in `*this` but not in `from`.
    //! 3. a call to `target.state(t, s)` for each term `t` of kind `s` stated
    //!    with `state() in `*this` but not in `from`.
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
    //! ```cpp
    //!    m.replay(from, &from);
    //! ```
    //!
    //! then `m == from` is guaranteed to hold.
    //!
    //! Note that a copy construction is sufficient to achieve the same
    //! equality. The purpose of this function is rather to be used with
    //! `target` objects of different types, as a way of observing the changes
    //! made to a module w.r.t. to `from`.
    //!
    //! This can also be used as a general iteration mechanism over all the
    //! constituent elements of the module (e.g. by providing an empty `from`
    //! module).
    //!
    //! If `target == nullptr`, the function does nothing.
    //!
    //! \note Declarations/definitions that are pending because of calls to
    //! declare() or define() with delayed resolution mode are not replayed by
    //! `replay`, until they are resolved with resolve().
    //!
    //! \note This function can be very efficient for the intended use cases,
    //! but the same caveat of the equality comparison operator apply, see the
    //! discussion above. For instance, if `*this` and `*from` are equal, but
    //! they are unrelated (i.e. one has not been obtained as a copy of the
    //! other), replay() may take some time to realize there is nothing to
    //! replay.
    void replay(module from, pipes::consumer *target) const;

    //!@}

  private:
    struct impl_t;
    support::lazy<support::boxed<impl_t>> _impl;
  };

  //!
  //! Internal representation of an entity declared or defined in a module.
  //!
  //! An \ref entity object represents an entity declared or defined in a
  //! module. Entity objects are stored internally in modules and are usually
  //! not handled explicitly during normal usage of the API.
  //!
  //! Entities are created in groups by collecting them at each call to
  //! resolve(), or declare()/define() with resolution::immediate. The lifetime
  //! of each entity in such groups is managed exclusively by \ref root objects
  //! whose pointers are returned by the corresponding `resolve()` call, and
  //! pointed by the `entity::root` field. Therefore as long as the \ref root
  //! object is alive, the corresponding \ref entity instances are safe to use.
  //!
  struct entity {
    //! The pointer to the root managing this entity object
    std::weak_ptr<struct root const> root; 
    
    variable name; //!< The name of the entity.
    types::type type; //!< The type of the entity.
    std::optional<term> value; //!< The value of the entity. 
                               //!< Empty if the entity is declared.
    std::optional<enum role> role; //!< The role of the entity
                                   //!< Empty if the entity is defined.

    //! \name Constructors
    //!@{

    //! Constructs an empty entity.
    entity() : name{label{}}, type{types::inferred()} { }

    //! Constructs the entity from a \ref decl.
    explicit entity(decl d) 
      : name{d.name}, type{d.type} { }
    
    //! Constructs the entity from a \ref def
    explicit entity(def d) 
      : name{d.name}, type{d.type}, value{d.value} { }
    
    
    entity(entity const&) = default;
    entity(entity &&) = default;
    
    entity &operator=(entity const&) = default;
    entity &operator=(entity &&) = default;

    bool operator==(entity const& other) const {
      return bool(name == other.name) && bool(type == other.type) 
          && value == other.value && root.lock() == other.root.lock();
    }

    //@}
  };

  //!
  //! A structure collecting instances of \ref entity created together.
  //!
  //! This is the root (hence the name) of ownership of \ref entity objects,
  //! since all the other references to them are weak (e.g. \ref object
  //! internally stores a weak pointer).
  //!
  //! Roots live in modules and are created by resolve() collecting all the
  //! pending declarations/definitions made to the module before.
  //!
  //!
  struct root
  {
    root() = default;

    root(root const&) = default;
    root(root &&) = default;
    
    root &operator=(root const&) = default;
    root &operator=(root &&) = default;

    //! whether the entities collected by this root are defined recursively or
    //! not.
    recursion mode = recursion::forbidden; 

    //! the entities collected by this root.
    std::vector<std::unique_ptr<entity const>> entities;
    
    //! the roots collecting all the entities referenced by the terms that
    //! defined our entities.
    std::vector<std::shared_ptr<root const>> dependencies;
  };

}

#endif // BLACK_LOGIC_SCOPE_HPP
