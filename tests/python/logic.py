#!/bin/env python3

from black_sat.logic import *
from typing import *

sigma = Alphabet()

p = sigma.symbol("p")
q = sigma.symbol("q")

u = Conjunction([p, q])

match u:
    case Conjunction(args):
        for a in args:
            print(a)
