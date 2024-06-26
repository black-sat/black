#!/bin/env python3

from black_sat.ast.core import *
from black_sat.logic import *
from typing import *

sigma = Alphabet()

l = Label(42)

assert l.value == 42

p = sigma.symbol(42)
ten = sigma.symbol(10)
q = sigma.symbol('q')

assert p.name == 42
assert q.name == 'q'

assert (p != q).__class__.__name__ == 'TermNEWrapper'

assert p != q

a = q(p - ten)

assert a.head == q
assert list(a.arguments) == [p - ten]

c = Conjunction([p, q == p])

assert c.arguments.__class__.__name__ == 'TermIterable'

match c:
    case Conjunction(args):
        assert list(args) == [sigma.symbol(42), Equal([q, p])]

print(c)
