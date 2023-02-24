# #!/usr/bin/env python3

# from black_sat import *

# def print_result(slv, f, result, *props):
#     print(f"formula: {f}")
#     if result == True:
#         print("Satisfiable")
#         n = slv.model.size
#         print(f"Model size: {n}")
#         for t in range(n):
#             for p in props:
#                 print(f"Value of proposition {p} at t = {t}: {slv.model.value(p, t)}")
#     elif result == False:
#         print("Unsatisfiable")
#     else:
#         print("Uknown")

# def report(str):
#     print(f"Syntax error: {str}")

# sigma = alphabet()

# x = sigma.variable("x")
# y = sigma.variable("y")

# ale = sigma.variable("ale")
# nicola = sigma.variable("nicola")
# luca = sigma.variable("luca")

# people = domain([ale, nicola, luca])

# person = sigma.named_sort("person")

# knows = sigma.relation("knows")

# f = ~knows(ale, luca) & forall([x[person], y[person]], knows(ale, x))

# xi = scope(sigma)
# xi.declare(person, people)
# xi.declare(knows, [person, person])

# assert xi.type_check(f, report)

# slv = solver()

# result1 = slv.solve(xi, f)
# result2 = slv.solve(xi, f)
# result3 = slv.solve(xi, ~f)

# assert not(result1 != result2)
# assert result2 != result3

# assert ~f == ~f

# print_result(slv, f, result1)

# p = sigma.proposition("p")
# q = sigma.proposition("q")

# f2 = p & X(p) & G(implies(p, q)) & F(~p)

# result4 = slv.solve(xi, f2)

# print_result(slv, f2, result4, p, q)

# f3 = G(p & F(q)) & F(~p & implies(q, q))
# result5 = slv.solve(xi, f3)

# print_result(slv, f3, result5)
# assert result5 == False

# print(f"Unsat core: {unsat_core(xi, f3)}")

# f4 = parse_formula(sigma, "p & F q")

# print(f"Parsed formula: {f4}")

# a = sigma.variable("a")
# b = sigma.variable("b")

# xi.declare(a, sigma.integer_sort())
# xi.declare(b, sigma.integer_sort())

# ff = (a == 0) & (b == 0) & G((a >= 0) & (b >= 0) & (a == 2 * b))

# print(ff)

# assert xi.type_check(ff)

# assert slv.solve(xi, ff, True, 500, True)

# assert slv.model.value(a == 0, 0)

# r = sigma.relation("r")

# xi.declare(r, [sigma.integer_sort()], scope.rigid)

# assert slv.solve(xi, r(0))

# assert slv.model.value(r(0), 0)

from black_sat import *

def report(err):
    print(f"Type error: {err}")
    quit()

sigma = alphabet()
xi = scope(sigma)

integer = sigma.integer_sort()

x = sigma.variable("x")
y = sigma.variable("y")
z = sigma.variable("z")
r = sigma.relation("r")

f = forall([x[integer], y[integer], z[integer]], implies(r(x, y, z), (x < z) & (z < y)))

g = (x == 0) & (y == 2) & G(r(x, y, z) & (wnext(z) == z + 1)) & F(z == 10)

print(g)

xi.declare(x, integer)
xi.declare(y, integer)
xi.declare(z, integer)
xi.declare(r, [integer, integer, integer], scope.rigid)

assert xi.type_check(f & g, report)

slv = solver()

assert slv.solve(xi, f & g, True)
assert slv.model is not None
last = slv.model.size - 1
print(f"Model size: {slv.model.size}")
print(f"y >= 11 at t = {last}: {slv.model.value(y >= 11, last)}")

p = sigma.proposition("p")
h = G(p) & F(~p)

assert not slv.solve(xi, h)

prova = big_and(sigma, [p,p,p,p,p])

print(prova)

prova = big_or(sigma, [p,p,p,p,p])

print(prova)
