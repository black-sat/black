# Scalable Category n.2
======================

## Structure of the formula

Let x_1 ... x_N a set of variables.

'''
x_1 > 1 &
/\_{i=1}^{N-2} X^{i-1}(next(x_{i+2}) = x_{i+1}+1) &
G(/\_{i=1}^{N} next(x_{i})=x_{i}) &
G( sum(x_1 ... x_N) < (N*(N+1))/2)
'''

where N is a parameter and is concretized in each formula.

## Command for BLACK

'''
./black solve -d reals -s ../benchmarks/formulas/ltlf-modulo-theory/LRA/scalable_2/scalable_2_N.ltlfmt
'''

## Expected Output

UNSAT
