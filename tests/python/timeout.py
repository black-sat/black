#!/usr/bin/env python3

from black_sat import *
from datetime import timedelta

sigma = alphabet()
xi = scope(sigma)

p = sigma.proposition("p")
x = sigma.variable("x")
xi.declare(x, sigma.integer_sort())

f1 = F(p) & G(~p)

f2 = F(x > 10) & G(x < 10)

slv = solver()

answer = slv.solve(xi, f1, finite = True, timeout = 3)

assert answer == False

answer = slv.solve(xi, f2, finite = True, timeout = 3)

assert answer is None