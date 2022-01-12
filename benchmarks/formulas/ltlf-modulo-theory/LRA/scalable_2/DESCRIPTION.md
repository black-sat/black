# Scalable Category n.2

## Structure of the formula

Variables: x1 ... xN

Formula:
```
e = 1 &
x = 0 & 
G(wnext(e) = e/2) &
G(wnext(x) = x + e) &
G(0 <= x < 2) &
F( 2-(1/10^N)< x)
```
for all N ∈ {1..18}.

## Properties

SAT

## Expected Output of BLACK

SAT

## Command for BLACK

```
./black solve -d reals -s ../benchmarks/formulas/ltlf-modulo-theory/LRA/scalable_2/scalable_2_N.ltlfmt
```
