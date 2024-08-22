#include <black/logic>
#include <iostream>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include <unordered_set>

using namespace black;
using namespace black::support;
using namespace black::logic;

using set_term = std::unordered_set<term, std::hash<term>, black::logic::term_equal_to<>>;
using set_term_ID = std::unordered_set<std::pair<term, int>, std::hash<term>, black::logic::term_equal_to<>>;

enum Surrogate { XS, WS, YS, ZS };

std::string term_to_string(term t);

/*
    Exercise 2.
*/
term push_negation(term t);
term to_nnf(term t);

/*
    Exercise 3.
*/
set_term set_union(set_term set1, set_term set2);
set_term quantify(std::vector<decl> q_vars, set_term vars);
set_term free_vars(term t);

/*
    Surrogates
*/
std::optional<set_term> closure(term t);
set_term closure_1(term t);
std::unordered_map<Surrogate, set_term> surrogates(term t);



std::string term_to_string(term t) {
    return match(t)(
        /*
            Object terms.
        */
        [](object obj)   {            
            if(obj.entity()->value.has_value()) { 
                return match(obj.entity()->value.value())(
                    [](integer i)  { return std::to_string(i.value()); },
                    [](real r)     { return std::to_string(r.value()); },                    
                    [](boolean b)  { return std::to_string(b.value()); },
                    [obj] (lambda) { return obj.entity()->name.name().to_string(); }
                );
            }
            return obj.entity()->name.name().to_string();
        },
        [](variable var) { return var.name().to_string(); },

        /*
            Boolean and first-order predicates.
        */
        [](equal, std::vector<term> arguments)      { return "(" + term_to_string(arguments[0]) + " == " + term_to_string(arguments[1]) + ")"; },
        [](distinct, std::vector<term> arguments)   { return "(" + term_to_string(arguments[0]) + " != " + term_to_string(arguments[1]) + ")"; },
        
        [](atom, term head, std::vector<term> arguments) {
            std::string result = term_to_string(head) + "(";
            for (auto i = arguments.begin(); i != arguments.end(); i ++) {
                if (i != arguments.begin()) result = result + ", ";
                result = result + term_to_string(*i);
            }
            return result + ")";
        }, 

        [](exists, std::vector<decl> decls, term body) {
            std::string result = "exists ";
            for (decl d : decls) {
                result = result + term_to_string(d.name().head()) + " ";
            }
            result = result + ". (" + term_to_string(body) + ")";
            return result;
        },
        [](forall, std::vector<decl> decls, term body) {
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
        [](negation, term argument) { return "!(" + term_to_string(argument) + ")"; },
        [](conjunction, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " && " + term_to_string(arguments[1]) + ")"; },
        [](disjunction, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " || " + term_to_string(arguments[1]) + ")"; },
        [](implication, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " => " + term_to_string(arguments[1]) + ")"; },

        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)         { return "X("  + term_to_string(argument) + ")"; },
        [](w_tomorrow, term argument)       { return "wX(" + term_to_string(argument) + ")"; },
        [](eventually, term argument)       { return "F("  + term_to_string(argument) + ")"; },
        [](always, term argument)           { return "G("  + term_to_string(argument) + ")"; },
        [](until, term left, term right)    { return "("  + term_to_string(left) + " U " + term_to_string(right) + ")"; },
        [](release, term left, term right)  { return "("  + term_to_string(left) + " R " + term_to_string(right) + ")"; },

        /*
            Future LTL operators.
        */
        [](yesterday, term argument)            { return "Y(" + term_to_string(argument) + ")"; },
        [](w_yesterday, term argument)          { return "Z(" + term_to_string(argument) + ")"; },
        [](once, term argument)                 { return "O(" + term_to_string(argument) + ")"; },
        [](historically, term argument)         { return "H(" + term_to_string(argument) + ")"; },
        [](since, term left, term right)        { return "(" + term_to_string(left) + " S " + term_to_string(right) + ")"; },
        [](triggered, term left, term right)    { return "(" + term_to_string(left) + " T " + term_to_string(right) + ")"; },

        /*
            Arithmetic operators.
        */
        [](minus, term argument)                { return "-(" + term_to_string(argument) + ")"; },
        [](sum, term left, term right)          { return "(" + term_to_string(left) + " + " + term_to_string(right) + ")"; },
        [](product, term left, term right)      { return "(" + term_to_string(left) + " * " + term_to_string(right) + ")"; },
        [](difference, term left, term right)   { return "(" + term_to_string(left) + " - " + term_to_string(right) + ")"; },
        [](division, term left, term right)     { return "(" + term_to_string(left) + " / " + term_to_string(right) + ")"; },

        /*
            Relational comparisons.
        */
        [](less_than, term left, term right)         { return "(" + term_to_string(left) + " < "  + term_to_string(right) + ")"; },
        [](less_than_eq, term left, term right)      { return "(" + term_to_string(left) + " <= " + term_to_string(right) + ")"; },
        [](greater_than, term left, term right)      { return "(" + term_to_string(left) + " > "  + term_to_string(right) + ")"; },
        [](greater_than_eq, term left, term right)   { return "(" + term_to_string(left) + " >= " + term_to_string(right) + ")"; }
    );
}

/*
    Given term t, returns !t in NNF.
*/
term push_negation(term t) {
    return match(t)(

        /*
            Boolean and first-order predicates.
        */
        [](equal, std::vector<term> arguments)      { return arguments[0] != arguments[1]; },
        [](distinct, std::vector<term> arguments)   { return arguments[0] == arguments[1]; },
        [](atom a) { return !a; },
        [](exists, std::vector<decl> decls, term body) { return forall(decls, to_nnf(!(body))); },
        [](forall, std::vector<decl> decls, term body) { return exists(decls, to_nnf(!(body))); },

        /*
            Boolean connectives.
        */
        [](negation, term argument)                  { return to_nnf(argument); },
        [](conjunction, std::vector<term> arguments) { return to_nnf(!(arguments[0])) || to_nnf(!(arguments[1])); },
        [](disjunction, std::vector<term> arguments) { return to_nnf(!(arguments[0])) && to_nnf(!(arguments[1])); },
        [](implication, std::vector<term> arguments) { return to_nnf(arguments[0]) && to_nnf(!(arguments[1])); },
        
        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)         { return X(to_nnf(!(argument))); },
        [](w_tomorrow, term argument)       { return wX(to_nnf(!(argument))); },
        [](eventually, term argument)       { return G(to_nnf(!(argument))); },
        [](always, term argument)           { return F(to_nnf(!(argument))); },
        [](until, term left, term right)    { return R(to_nnf(!left), to_nnf(!right)); },
        [](release, term left, term right)  { return U(to_nnf(!left), to_nnf(!right)); },

        /*
            Past LTL operators.
        */
        [](yesterday, term argument)            { return Y(to_nnf(!(argument))); },
        [](w_yesterday, term argument)          { return Z(to_nnf(!(argument))); },
        [](once, term argument)                 { return H(to_nnf(!(argument))); },
        [](historically, term argument)         { return O(to_nnf(!(argument))); },
        [](since, term left, term right)        { return T(to_nnf(!left), to_nnf(!right)); },
        [](triggered, term left, term right)    { return S(to_nnf(!left), to_nnf(!right)); },

        /*
            Relational comparisons.
        */
        [](less_than, term left, term right)         { return left >= right; },
        [](less_than_eq, term left, term right)      { return left > right;  },
        [](greater_than, term left, term right)      { return left <= right; },
        [](greater_than_eq, term left, term right)   { return left < right;  },

        [](boolean b) { return !b; }
    );
}

/*
    Given a term t, returns t in NNF.
*/
term to_nnf(term t) {
    return match(t)(
        /*
            If t is a negation, the negation is pushed inside.
        */
        [](negation, term argument) { return push_negation(argument); },

        /*
            Object terms.
        */
        [](object obj)   { return obj; },
        [](variable var) { return var; },

        /*
            Boolean and first-order predicates
        */
        [](equal eq)     { return eq; },
        [](distinct in)  { return in; },
        [](atom a)       { return a; },
        [](exists, std::vector<decl> decls, term body) { return exists(decls, to_nnf(body)); },
        [](forall, std::vector<decl> decls, term body) { return forall(decls, to_nnf(body)); },

        /*
            Boolean connectives (not negation).
        */
        [](conjunction, std::vector<term> arguments) { return to_nnf(arguments[0]) && to_nnf(arguments[1]); },
        [](disjunction, std::vector<term> arguments) { return to_nnf(arguments[0]) || to_nnf(arguments[1]); },
        [](implication, std::vector<term> arguments) { return implication(to_nnf(arguments[0]), to_nnf(arguments[1])); },

        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)         { return X(to_nnf(argument)); },
        [](w_tomorrow, term argument)       { return wX(to_nnf(argument)); },
        [](eventually, term argument)       { return F(to_nnf(argument)); },
        [](always, term argument)           { return G(to_nnf(argument)); },
        [](until, term left, term right)    { return U(to_nnf(left), to_nnf(right)); },
        [](release, term left, term right)  { return R(to_nnf(left), to_nnf(right)); },

        /*
            Past LTL operators.
        */
        [](yesterday, term argument)            { return Y(to_nnf(argument)); },
        [](w_yesterday, term argument)          { return Z(to_nnf(argument)); },
        [](once, term argument)                 { return O(to_nnf(argument)); },
        [](historically, term argument)         { return H(to_nnf(argument)); },
        [](since, term left, term right)        { return S(to_nnf(left), to_nnf(right)); },
        [](triggered, term left, term right)    { return T(to_nnf(left), to_nnf(right)); },

        /* 
            Arithmetic operators.
        */
        [](minus m)         { return m; },
        [](sum s)           { return s; },
        [](product p)       { return p; },
        [](difference diff) { return diff; },
        [](division div)    { return div; },

        /*
            Relational comparisons.
        */
        [](less_than, term left, term right)         { return left < right; },
        [](less_than_eq, term left, term right)      { return left <= right; },
        [](greater_than, term left, term right)      { return left > right; },
        [](greater_than_eq, term left, term right)   { return left >= right; }
    );
}

/*
    Exercise 3. Free variables.
*/

set_term set_union(set_term set1, set_term set2) {
    set1.insert (set2.begin(), set2.end());
    return set1;
}
set_term set_union(set_term set1, set_term set2, set_term set3) {
    set1 = set_union(set1, set2);
    return set_union(set1, set3);
}

/*
    Removes any quantified variable (q_vars) from the set of free variables vars.
*/
set_term quantify(std::vector<decl> q_vars, set_term vars) {
    for (decl d : q_vars) {
        vars.erase(vars.find(d.name));
    }
    return vars;
}

set_term free_vars(term t) { 
    return match(t)(
        /*
            Base cases. In case of an object, distinguish between declared and defined objects.
        */
        [](variable var) -> set_term { return { var }; },
        [](object)   -> set_term { return { }; },

        /*
            Quantifiers.
        */
        [](exists, std::vector<decl> decls, term body) -> set_term { return quantify(decls, free_vars(body)); },
        [](forall, std::vector<decl> decls, term body) -> set_term { return quantify(decls, free_vars(body)); },
        
        /*
            Boolean connectives.
        */
        [](negation, term argument)                  -> set_term { return free_vars(argument); },
        [](conjunction, std::vector<term> arguments) -> set_term { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](disjunction, std::vector<term> arguments) -> set_term { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](implication, std::vector<term> arguments) -> set_term { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },

        /*
            Boolean and first-order predicates
        */
        [](equal, std::vector<term> arguments)    -> set_term   { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](distinct, std::vector<term> arguments) -> set_term   { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](atom, std::vector<term> arguments)     -> set_term   { 
                                                                    set_term fv = { };
                                                                    for (term t : arguments){
                                                                        fv = set_union(fv, free_vars(t));
                                                                    }
                                                                    return fv;
                                                                },

        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)        -> set_term { return free_vars(argument); },
        [](w_tomorrow, term argument)      -> set_term { return free_vars(argument); },
        [](eventually, term argument)      -> set_term { return free_vars(argument); },
        [](always, term argument)          -> set_term { return free_vars(argument); },
        [](until, term left, term right)   -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](release, term left, term right) -> set_term { return set_union(free_vars(left), free_vars(right)); },

        /*
            Past LTL operators.
        */
        [](yesterday, term argument)            -> set_term { return free_vars(argument); },
        [](w_yesterday, term argument)          -> set_term { return free_vars(argument); },
        [](once, term argument)                 -> set_term { return free_vars(argument); },
        [](historically, term argument)         -> set_term { return free_vars(argument); },
        [](since, term left, term right)        -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](triggered, term left, term right)    -> set_term { return set_union(free_vars(left), free_vars(right)); },
        
        /*
            Arithmetic operators.
        */
        [](minus, term argument)                -> set_term { return free_vars(argument); },
        [](sum, term left, term right)          -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](product, term left, term right)      -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](difference, term left, term right)   -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](division, term left, term right)     -> set_term { return set_union(free_vars(left), free_vars(right)); },

        /*
            Relational comparisons
        */
        [](less_than, term left, term right)        -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](less_than_eq, term left, term right)     -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than, term left, term right)     -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than_eq, term left, term right)  -> set_term { return set_union(free_vars(left), free_vars(right)); }
    );
}


/*
    Given term t in NNF, returns t in SNF.
*/
term to_snf(term t) {
    return match(t)(

        [](object obj)   { return obj; },
        [](variable var) { return var; },
        /*
            Boolean and first-order predicates.
        */
        [](equal e)     { return e; },
        [](distinct d)  { return d; },
        [](atom a)      { return a; },
        [](exists, std::vector<decl> decls, term body) { return exists(decls, to_snf(body)); },
        [](forall, std::vector<decl> decls, term body) { return forall(decls, to_snf(body)); },

        /*
            Boolean connectives.
        */
        [](negation, term argument)                  { return !to_snf(argument); },
        [](conjunction, std::vector<term> arguments) { return to_snf(arguments[0]) && to_snf(arguments[1]); },
        [](disjunction, std::vector<term> arguments) { return to_snf(arguments[0]) || to_snf(arguments[1]); },
        [](implication, std::vector<term> arguments) { return implication(to_snf(arguments[0]), to_snf(arguments[1])); },
        
        /*
            Future LTL operators.
        */
        [](tomorrow t) { return t; },
        [](w_tomorrow wt) { return wt; },
        [](eventually f, term argument)       { return to_snf(argument) || X(f); },
        [](always a, term argument)           { return to_snf(argument) && wX(a); },
        [](until u, term left, term right)    { return to_snf(right) || (to_snf(left) && X(u)); },
        [](release r, term left, term right)  { return to_snf(right) && (to_snf(left) || wX(r)); },

        /*
            Past LTL operators.
        */
        [](yesterday y)            { return y; },
        [](w_yesterday z)          { return z; },
        [](once o, term argument)                 { return to_snf(argument) || Y(o); },
        [](historically h, term argument)         { return to_snf(argument) && Z(h); },
        [](since s, term left, term right)        { return to_snf(right) || (to_snf(left) && Y(s)); },
        [](triggered t, term left, term right)    { return to_snf(right) && (to_snf(left) && Z(t)); },

        /* 
            Arithmetic operators.
        */
        [](minus m)         { return m; },
        [](sum s)           { return s; },
        [](product p)       { return p; },
        [](difference diff) { return diff; },
        [](division div)    { return div; },
        
        /*
            Relational comparisons.
        */
        [](less_than lt)            { return lt; },
        [](less_than_eq lte)        { return lte;  },
        [](greater_than gt)         { return gt; },
        [](greater_than_eq gte)     { return gte;  }
    );
}


/*
    Given a FOLTL formula t, returns the closure of t. That is, X(t) and the rest of the closure.
*/
std::optional<set_term> closure(term t) {
    return match(t)(
        [](minus)       -> std::optional<set_term> { throw std::invalid_argument( "first-order term has no closure."); return std::nullopt; },
        [](sum)         -> std::optional<set_term> { throw std::invalid_argument( "first-order term has no closure."); return std::nullopt; },
        [](product)     -> std::optional<set_term> { throw std::invalid_argument( "first-order term has no closure."); return std::nullopt; },
        [](division)    -> std::optional<set_term> { throw std::invalid_argument( "first-order term has no closure."); return std::nullopt; },
        [](difference)  -> std::optional<set_term> { throw std::invalid_argument( "first-order term has no closure."); return std::nullopt; },
        [](object)      -> std::optional<set_term> { throw std::invalid_argument( "first-order term has no closure."); return std::nullopt; },
        [](variable)    -> std::optional<set_term> { throw std::invalid_argument( "first-order term has no closure."); return std::nullopt; },
    
        [](term t)      -> std::optional<set_term> { return set_union({ X(t) }, closure_1(t)); }
    );
}

set_term closure_1(term t) {
    return match(t)(
        [](object)      -> set_term { return { }; },
        [](variable)    -> set_term { return { }; },

        /*
            Boolean and first-order predicates
        */
        [](equal e)    -> set_term { return { e }; },
        [](distinct d) -> set_term { return { d }; },
        [](atom a)     -> set_term { return { a }; },
        [](exists e, std::vector<decl>, term argument) -> set_term { return set_union( { e }, closure_1(argument) ); },
        [](forall f, std::vector<decl>, term argument) -> set_term { return set_union( { f }, closure_1(argument) ); },

        /*
            Boolean connectives.
        */
        [](negation n, term argument)                  -> set_term { return set_union({ n }, closure_1(argument)); },
        [](conjunction c, std::vector<term> arguments) -> set_term { return set_union(closure_1(arguments[0]), closure_1(arguments[1]), {c}); },
        [](disjunction d, std::vector<term> arguments) -> set_term { return set_union(closure_1(arguments[0]), closure_1(arguments[1]), {d}); },
        [](implication i, term left, term right)       -> set_term { return set_union(closure_1(left), closure_1(right), {i}); },

        /*
            Future LTL operators.
        */
        [](tomorrow t, term argument)        -> set_term { return set_union({ t }, closure_1(argument)); },
        [](w_tomorrow wt, term argument)     -> set_term { return set_union({ wt }, closure_1(argument)); },
        [](eventually f, term argument)      -> set_term { return set_union({ f }, closure_1(argument)); },
        [](always g, term argument)          -> set_term { return set_union({ g }, closure_1(argument)); },
        [](until u, term left, term right)   -> set_term { return set_union({ u, X(u) }, closure_1(left), closure_1(right)); },
        [](release r, term left, term right) -> set_term { return set_union({ r, wX(r) }, closure_1(left), closure_1(right)); },

        /*
            Past LTL operators.
        */
        [](yesterday y, term argument)         -> set_term { return set_union({ y }, closure_1(argument)); },
        [](w_yesterday z, term argument)       -> set_term { return set_union({ z }, closure_1(argument)); },
        [](once o, term argument)              -> set_term { return set_union({ o }, closure_1(argument)); },
        [](historically h, term argument)      -> set_term { return set_union({ h }, closure_1(argument)); },
        [](since s, term left, term right)     -> set_term { return set_union({ s, Y(s) }, closure_1(left), closure_1(right)); },
        [](triggered t, term left, term right) -> set_term { return set_union({ t, Z(t) }, closure_1(left), closure_1(right)); },

        /*
            Arithmetic operators.
        */
        [](minus)      -> set_term { return { }; },
        [](sum)        -> set_term { return { }; },
        [](product)    -> set_term { return { }; },
        [](difference) -> set_term { return { }; },
        [](division)   -> set_term { return { }; },

        /*
            Relational comparisons.
        */
        [](less_than lt)        -> set_term { return { lt }; },
        [](less_than_eq lte)    -> set_term { return { lte }; },
        [](greater_than gt)     -> set_term { return { gt }; },
        [](greater_than_eq gte) -> set_term { return { gte }; }
    );
}

std::string id_to_string(term t) {
    return std::to_string((uintptr_t) t.unique_id());
}

/*std::vector<decl> fv_to_vec(set_term vars) {
    std::vector<decl> result;
    for(variable t : vars) {
        result.insert(t);
    }
    return result;
}*/

/*std::unordered_map<Surrogate, set_term> surrogates(term t) {
    std::unordered_map<Surrogate, set_term> result;
    result[XS] = { };
    result[WS] = { };
    result[YS] = { };
    result[ZS] = { };

    int i = 0;

    for (term t1 : closure(to_snf(to_nnf(t)))) {
        match(t1)(
            [](tomorrow to){
                set_term fv = free_vars(to);

            }[](w_tomorrow wto){
                result[WS].insert(mod.define(
                    i, {}, types::boolean(),
                    wto,
                    resolution::immediate
                ));
            }[](yesterday y){
                result[YS].insert(mod.define(
                    i, {}, types::boolean(),
                    y,
                    resolution::immediate
                ));
            }[](w_yesterday z){
                result[ZS].insert(mod.define(
                    i, {}, types::boolean(),
                    z,
                    resolution::immediate
                ));
            }
        );
    }

    return result; 
}*/