#include <black/logic>
#include <iostream>
#include <vector>

using namespace black;
using namespace black::support;
using namespace black::logic;

term to_nnf(term t);
std::string object_to_string(object obj);

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
        [](atom a) { return a; },
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
            Relational comparisons.
        */
        [](less_than, term left, term right)         { return left < right; },
        [](less_than_eq, term left, term right)      { return left <= right;  },
        [](greater_than, term left, term right)      { return left > right; },
        [](greater_than_eq, term left, term right)   { return left >= right;  }
    );
}

std::string term_to_string(term t) {
    return match(t)(
        /*
            Object terms.
        */
        [](object obj)   { return object_to_string(obj); },
        [](variable var) { return var.name().to_string(); },

        /*
            Boolean and first-order predicates.
        */
        [](equal, std::vector<term> arguments)      { return "(" + term_to_string(arguments[0]) + " == " + term_to_string(arguments[1]) + ")"; },
        [](distinct, std::vector<term> arguments)   { return "(" + term_to_string(arguments[0]) + " != " + term_to_string(arguments[1]) + ")"; },
        
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
    If obj is a constant, return its value. If obj is a variable, return its label name.
*/
std::string object_to_string(object obj) {
    try {
        return match(obj.entity()->value.value())(
            [](integer i)   { return std::to_string(i.value()); },
            [](real r)      { return std::to_string(r.value()); },
            [](boolean b)   { return std::to_string(b.value()); }
        );
    }
    catch(std::bad_optional_access const&) {
        return obj.entity()->name.name().to_string();
    }
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
    

    return 0;
}