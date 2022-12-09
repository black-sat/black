#!/usr/bin/env python3

from black import *

sigma = alphabet()

x = sigma.variable("x")
y = sigma.variable("y")

ale = sigma.variable("ale")
nicola = sigma.variable("nicola")
luca = sigma.variable("luca")

people = domain([ale, nicola, luca])

person = sigma.named_sort("person")

knows = sigma.relation("knows")

f = ~knows(ale, luca) & forall([x[person], y[person]], knows(ale, x))

xi = scope(sigma)
xi.declare(person, people)
xi.declare(knows, [person, person])

slv = solver()

result = slv.solve(xi, f)
result2 = slv.solve(xi, f)
result3 = slv.solve(xi, ~f)

assert not(result != result2)
assert result2 != result3

assert ~f == ~f

print(f"formula: {f}")
if result == True:
    print("Satisfiable")
elif result == False:
    print("Unsatisfiable")
else:
    print("Uknown")

