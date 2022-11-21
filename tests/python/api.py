#!/usr/bin/env python3

from black import *

sigma = alphabet()

x = sigma.variable("x")
y = sigma.variable("y")

e = equal([x, y])

s = sigma.integer_sort()

f = exists([x[s], y[s]], e)

print(f)

t1 = (x + y)
t2 = (x * y)

f = G(U(t1 < t2, t2 == t1))

print(f)