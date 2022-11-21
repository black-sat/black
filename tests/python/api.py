#!/usr/bin/env python3

from black import *

sigma = alphabet()

x = sigma.variable("x")
y = sigma.variable("y")

e = equal([x, y])

x_decl = sigma.var_decl(x, sigma.integer_sort())
y_decl = sigma.var_decl(y, sigma.integer_sort())

f = exists([x_decl, y_decl], e)

print(f)

t1 = (x + y)
t2 = (x * y)

f = G(U(t1 < t2, t2 == t1))

print(f)