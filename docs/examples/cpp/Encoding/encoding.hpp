std::vector<variable>::iterator find_var(std::vector<variable> &, variable);
std::vector<variable> quantify(std::vector<decl>, std::vector<variable>);
types::type type_from_module(module, variable);
module surrogates(term, black::pipes::consumer *);
std::vector<variable> surrogates(module &, module &, term, black::pipes::consumer *);

/*
    Returns an iterator pointing to the first occurrence of target in vec.
*/
std::vector<variable>::iterator find_var(std::vector<variable> &vec, variable target) {
    return std::find_if(vec.begin(), vec.end(), [target](variable var) {
        return term_equal(target, var);
    });
}

std::vector<variable> vec_union(std::vector<variable> vec1, std::vector<variable> vec2) {
    for(variable var2: vec2) {
        if(find_var(vec1, var2) == vec1.end()) {
            vec1.push_back(var2);
        }
    }
    return vec1;
}

std::vector<variable> quantify(std::vector<decl> q_vars, std::vector<variable> vars) {
    for (decl d : q_vars) {
        vars.erase(find_var(vars, d.name));
    }
    return vars;
}

types::type type_from_module(module mod, variable var) {
    return mod.lookup(var).value().entity()->type;
}

module surrogates(term t, black::pipes::consumer *_next){
    module gamma, aux;
    object xs_phi = gamma.declare({"XPHI", types::boolean()}, resolution::delayed);

    // (iii)
    _next->state(xs_phi,         logic::statement::init);
    _next->state(t == xs_phi,    logic::statement::transition);
    _next->state(!xs_phi,        logic::statement::final);
    surrogates(gamma, aux, t, _next);
    return gamma;
}

std::vector<variable> surrogates(module &gamma, module &aux, term t, black::pipes::consumer *_next) {
    return match(t)(
        /*
            Base cases.
        */
        [](variable var) -> std::vector<variable> { return { var }; },
        [](object)       -> std::vector<variable> { return { }; },

        /*
            Quantifiers. All quantified variables are stored (declared) in a new module aux2. The recursive relies on aux2 to retrieve the types
            of all free variables in term body.
        */
        [&](exists, std::vector<decl> decls, term body) -> std::vector<variable> {
            module aux2 = aux;
            for(decl d : decls) {
                aux2.declare(d, resolution::immediate);
            }
            // Ritorna le variabili libere in body, rimuovendo quelle quantificate in decls.
            return quantify(decls, surrogates(gamma, aux2, body, _next));
        },
        [&](forall, std::vector<decl> decls, term body) -> std::vector<variable> {
            module aux2 = aux;
            for(decl d : decls) {
                aux2.declare(d, resolution::immediate);
            }
            // Ritorna le variabili libere in body, rimuovendo quelle quantificate in decls.
            return quantify(decls, surrogates(gamma, aux2, body, _next));
        },

        /*
            Temporal operators.
                (i) Free variables in term body are retrieved;
                (ii) The surrogate is defined accordingly;
                (iii) The conjuncts containing the surrogate are passed to the next stage in the pipeline.
        */
        [&](tomorrow to, term body) -> std::vector<variable> {
            
            // (i)
            std::vector<variable> free_vars = surrogates(gamma, aux, body, _next);
            std::vector<term> free_terms = { };
            std::vector<decl> decls = { };

            // Retrieve free variable types.
            std::vector<types::type> types = { };
            for(variable var : free_vars){
                types::type t = type_from_module(aux, var);
                decls.push_back({var, t});
                types.push_back(t);
                free_terms.push_back(var);
            }

            // (ii)
            object surr = gamma.declare(
                {"XS", types::function(types, types::boolean())},
                resolution::delayed
            );
            term a = atom(surr, free_terms);

            // (iii)
            _next->state(forall(decls, to == a),   logic::statement::transition);
            _next->state(forall(decls, !a),         logic::statement::final);

            return free_vars;
        },
        [&](w_tomorrow w_to, term body) -> std::vector<variable> {
            
            // (i)
            std::vector<variable> free_vars = surrogates(gamma, aux, body, _next);
            std::vector<term> free_terms = { };
            std::vector<decl> decls = { };

            // Retrieve free variable types.
            std::vector<types::type> types = { };
            for(variable var : free_vars){
                types::type t = type_from_module(aux, var);
                decls.push_back({var, t});
                types.push_back(t);
                free_terms.push_back(var);
            }

            // (ii)
            object surr = gamma.declare(
                {"XW", types::function(types, types::boolean())},
                resolution::delayed
            );
            term a = atom(surr, free_terms);

            // (iii)
            _next->state(forall(decls, w_to == a),   logic::statement::transition);
            _next->state(forall(decls, a),         logic::statement::final);

            return free_vars;
        },

        [&](yesterday y, term body) -> std::vector<variable> {
            
            // (i)
            std::vector<variable> free_vars = surrogates(gamma, aux, body, _next);
            std::vector<term> free_terms = { };
            std::vector<decl> decls = { };

            // Retrieve free variable types.
            std::vector<types::type> types = { };
            for(variable var : free_vars){
                types::type t = type_from_module(aux, var);
                decls.push_back({var, t});
                types.push_back(t);
                free_terms.push_back(var);
            }

            // (ii)
            object surr = gamma.declare(
                {"XY", types::function(types, types::boolean())},
                resolution::delayed
            );
            term a = atom(surr, free_terms);

            // (iii)
            _next->state(forall(decls, y == a),   logic::statement::transition);
            _next->state(forall(decls, !y),         logic::statement::init);

            return free_vars;
        },

        [&](w_yesterday z, term body) -> std::vector<variable> {
            // (i)
            std::vector<variable> free_vars = surrogates(gamma, aux, body, _next);
            std::vector<term> free_terms = { };
            std::vector<decl> decls = { };

            // Retrieve free variable types.
            std::vector<types::type> types = { };
            for(variable var : free_vars){
                types::type t = type_from_module(aux, var);
                decls.push_back({var, t});
                types.push_back(t);
                free_terms.push_back(var);
            }

            // (ii)
            object surr = gamma.declare(
                {"XY", types::function(types, types::boolean())},
                resolution::delayed
            );
            term a = atom(surr, free_terms);

            // (iii)
            _next->state(forall(decls, z == a),   logic::statement::transition);
            _next->state(forall(decls, z),         logic::statement::init);

            return free_vars;
        },

        /* 
            Boolean and first-order predicates
        */
        [&](equal, std::vector<term> arguments) -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], _next), surrogates(gamma, aux, arguments[1], _next)); },
        [&](distinct, std::vector<term> arguments) -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], _next), surrogates(gamma, aux, arguments[1], _next)); },
        [&](atom, std::vector<term> arguments) -> std::vector<variable> { 
            std::vector<variable> result = { };
            for(term t : arguments) {
                result = vec_union(result, surrogates(gamma, aux, t, _next));
            }
            return result;
         },

        /*
            Boolean connectives.
        */
        [&](negation, term argument)                    -> std::vector<variable> { return surrogates(gamma, aux, argument, _next); },
        [&](conjunction, std::vector<term> arguments)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], _next), surrogates(gamma, aux, arguments[1], _next)); },
        [&](disjunction, std::vector<term> arguments)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], _next), surrogates(gamma, aux, arguments[1], _next)); },
        [&](implication, term left, term right)         -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },

        /*
            Other temporal operators.
        */
        [&](eventually, term argument)          -> std::vector<variable> { return surrogates(gamma, aux, argument, _next); },
        [&](always, term argument)              -> std::vector<variable> { return surrogates(gamma, aux, argument, _next); },
        [&](until, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](release, term left, term right)     -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](once, term argument)                -> std::vector<variable> { return surrogates(gamma, aux, argument, _next); },
        [&](historically, term argument)        -> std::vector<variable> { return surrogates(gamma, aux, argument, _next); },
        [&](since, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](triggered, term left, term right)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },

        /* 
            Arithmetic operators.
        */
        [&](minus, term argument)                -> std::vector<variable> { return surrogates(gamma, aux, argument, _next); },
        [&](sum, term left, term right)          -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](product, term left, term right)      -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](difference, term left, term right)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](division, term left, term right)     -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },

        /*
            Relational comparisons.
        */
        [&](less_than, term left, term right)          -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](less_than_eq, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](greater_than, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); },
        [&](greater_than_eq, term left, term right)    -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, _next), surrogates(gamma, aux, right, _next)); }
    );
}