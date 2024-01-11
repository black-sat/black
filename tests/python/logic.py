#!/bin/env python3

from black_sat.ast.core import *
from black_sat.logic import *
from typing import *

sigma = Alphabet()

l = Label(42)

assert l.value == 42

p = sigma.symbol(42)
q = sigma.symbol('q')

assert p.name == 42
assert q.name == 'q'

assert p != q

c = Conjunction([p, q])

assert c.arguments.__class__.__name__ == 'TermIterable'

match c:
    case Conjunction(args):
        l = [a.name for a in args]
        assert l == [42, 'q']
