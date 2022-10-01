# Scalable Category n.3

## Structure of the formula

Variables: x1 ... xN

Formula:
```
x0 > 0 &
/\_{i=0}^{n-1} X^i ( next(x_{i+1}) > x_i ) & 
G( /\_{i=0}^{n} wnext(x_i) = x_i ) & 
G(sum(x0 ... x_{n-1}) = n\*(n+1)/2 - 1)
```

## Properties

UNSAT

- For dimension N, BLACK terminates with UNSAT at iteration N.

- Backward Forcing: only when arrived at iteration N, the solver decides the
  correct value for all the variable in previous time points.

- In all iterations K < N, the solver doesn't learn nothing that could use in
  iteration K+1.

- The family of formulas grows linearly with respect to N.


## Expected Output of BLACK

UNSAT

## Command for BLACK

```
./black solve -d Int -s ../benchmarks/formulas/ltlf-modulo-theory/LIA/scalable_3/scalable_3_N.ltlfmt
```

