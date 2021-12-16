# Scalable Category n.1

## Structure of the formula

Variables: x1 ... xN

Formula:
```
x=0 & G(wnext(x) = x+1) & F(x=N)
```

## Properties

SAT

- For dimension N, BLACK terminates with SAT at iteration N.

- The explicit-state tableau is exponential in |phi| (recall that N is
  represented in binary): it contains at least a branch of length N.
  BLACK seems not to suffer from this exponential growth.

- The family of formulas grows logarithimcally with respect to N.


## Expected Output of BLACK

UNSAT

## Command for BLACK

```
./black solve -d integers -s ../benchmarks/formulas/ltlf-modulo-theory/LIA/scalable_1/scalable_1_N.ltlfmt
```

