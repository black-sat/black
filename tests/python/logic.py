#!/bin/env python3

from black_sat import *

sigma = alphabet()

p = sigma.symbol("p")
q = sigma.symbol("q")

c = conjunction([p, q])

match c:
    case conjunction:
        print("Hello")