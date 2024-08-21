#include <black/logic>
#include <iostream>
#include <vector>
#include "utils.hpp"

using namespace black;
using namespace black::support;
using namespace black::logic;

term to_nnf(term t);
term push_negation(term t);

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

int main() {
    
    module mod;

    object p = mod.declare("p", types::boolean());
    object q = mod.declare("q", types::boolean());

    object x = mod.declare("x", types::integer());
    object c = mod.define("c", types::integer(), 3);

    term t1 = !(!(p && (x > c)));
    term t2 = !(X(p && p));
    term t3 = !(p && q);

    std::cout << "t1: " << term_to_string(t1) << std::endl;
    std::cout << "t2: " << term_to_string(t2) << std::endl;
    std::cout << "t3: " << term_to_string(t3) << std::endl;

    std::cout << std::endl;

    ////////////////////// TEST 2 //////////////////////   

    variable y = "y";
    variable z = "z";

    term e = exists({{y, types::integer()}}, y > z);
    term f = !(F(G (forall({{z, types::integer()}}, e))));

    std::cout << "Term f: " << term_to_string(f) << std::endl; 
    std::cout << "Term f (NNF): " << term_to_string(to_nnf(f)) << std::endl;

    std::cout << std::endl;

    //// TEST 3 ///
    term expr = x + c;
    std::cout << "Term expr: " << term_to_string(expr) << std::endl; 
    std::cout << "Term expr (NNF): " << term_to_string(to_nnf(expr)) << std::endl; 

    /// TEST 4: Atoms ///

    std::cout << std::endl;

    variable P = "P";
    object a = mod.define(
        P, {{y, types::boolean()}, {z, types::boolean()}}, types::boolean(),
        y && z,
        resolution::immediate
    );
    term u = !(a(p, q));
    std::cout << "Term not P(p, q): " << term_to_string(u) << std::endl; 
    std::cout << "Term not P(p, q) (NNF): " << term_to_string(to_nnf(u)) << std::endl; 

    return 0;
}