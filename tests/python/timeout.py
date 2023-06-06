#!/usr/bin/env python3

from black_sat import *
from datetime import timedelta

sigma = alphabet()
xi = scope(sigma)

x = sigma.variable("x")
xi.declare(x, sigma.integer_sort())

f = F(x > 10) & G(x < 10)

slv = solver()

answer = slv.solve(xi, f, finite = True, timeout = 3)

assert answer is None