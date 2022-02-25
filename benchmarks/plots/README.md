# Plots

How to use the scripts to generate the plots from raw data.

You can see the meaning of each command line argument with, for example, `python3 make-survival-plot.py -h`.

## Scatter plots

`$ python3 make-scatter-plot.py ../results/jar/finite-210811.dat aalta/finite black/finite 400 --pdf --html --log`

`$ python3 make-scatter-plot.py ../results/jar/future-210817.dat aalta/v2 black/mathsat 400 --pdf --html --log`

## Survival plots

`$ python3 make-survival-plot.py ../results/jar/future-210817.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph" -p -m -i 10000 --both`

`$ python3 make-survival-plot.py ../results/jar/future-210817.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph" -p -m -i 10000 --both --family acacia`

`$ python3 make-survival-plot.py ../results/jar/past-210818.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive" -p -m -i 10000 --both`

`$ python3 make-survival-plot.py ../results/jar/finite-210818.dat "black/finite, aalta/finite, ltl2sat/default" -p -m -i 10000 --both`
