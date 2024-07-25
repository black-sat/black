#include <black/logic>
#include <iostream>
#include <vector>

using namespace black;
using namespace black::support;
using namespace black::logic;

term to_nnf(term t);
std::string object_to_string(object obj);

term push_negation(term t) {
    return match(t)(
        /*
            CASE: !(!(t)). RETURNS NNF OF: t.
        */
        [](negation, term argument) {
            std::cout << "!negation" << std::endl;
            return to_nnf(argument);
        },

        /*
            CASE: !(t1 && t2). RETURNS NNF OF: (!t1 || !t2).
        */
        [](conjunction, std::vector<term> arguments) {
            std::cout << "!conjunction" << std::endl;
            return to_nnf(!(arguments[0])) || to_nnf(!(arguments[1]));
        },
        
        /*
            CASE: !(t1 || t2). RETURNS NNF OF: (!t1 && !t2).
        */
        [](disjunction, std::vector<term> arguments) {
            return to_nnf(!(arguments[0])) && to_nnf(!(arguments[1]));
        },
        
        /*
            CASE: !(t1 => t2). RETURNS NNF OF: (t1 && !t2).
        */
        [](implication, std::vector<term> arguments) {
            return to_nnf(arguments[0]) && to_nnf(!(arguments[1]));
        },

        /*
            CASE: !(X(t)). RETURNS NNF OF: X(!(t)).
        */
        [](tomorrow, term argument) {
            return X(to_nnf(!(argument)));
        },

        [](object obj) {
            return !obj;
        }
    );
}

term to_nnf(term t) {
    return match(t)(
        [](negation, term argument) {
            std::cout << "Negation" << std::endl;
            return push_negation(argument);
        },
        []<typename T>(T, std::vector<term> arguments) {
            std::cout << "Conjunction" << std::endl;
            return T({
                to_nnf(arguments[0]),
                to_nnf(arguments[1])
            });
        },
        [](object obj) { return obj; }
    );
}

std::string term_to_string(term t) {
    return match(t)(
        /*
            Object terms.
        */
        [](object obj)  { return object_to_string(obj); },
        [](variable var){ return var.name().to_string(); },
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

    return 0;
}