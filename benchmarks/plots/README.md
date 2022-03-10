# Plots

How to use the scripts to generate the plots from raw data.

You can see the meaning of each command line argument with, for example, `python3 make-survival-plot.py -h`.

## Scatter plots

These are the commands used to generate the plots in the paper. The commands are repeated for each tool that we compared with BLACK.

`$ python3 make-scatter-plot.py ../results/jar/future-210817.dat aalta/v2 black/mathsat 400 --pdf --html --log`

`$ python3 make-scatter-plot.py ../results/jar/past-210818.dat nuXmv/sbmc black/mathsat 400 --pdf --html --log`

`$ python3 make-scatter-plot.py ../results/jar/finite-210818.dat aalta/finite black/finite 400 --pdf --html --log`

## Survival plots

These are the commands used to generate the plots in the paper.

`$ python3 make-survival-plot.py ../results/jar/future-210817.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph" -p -m -i 10000 --both`

`$ python3 make-survival-plot.py ../results/jar/past-210818.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive" -p -m -i 10000 --both`

`$ python3 make-survival-plot.py ../results/jar/finite-210818.dat "black/finite, aalta/finite, ltl2sat/default" -p -m -i 10000 --both`

One can also filter the survival plots by family with, for example, the following command.

`$ python3 make-survival-plot.py ../results/jar/future-210817.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph" -p -m -i 10000 --both --family acacia`

## Bar charts

These are the commands used to generate the plots in the paper.

`python3 make-bar-charts.py ../results/jar/future-210817.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph" -p -m`

`python3 make-bar-charts.py ../results/jar/past-210818.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive" -p -m -w 500`

`python3 make-bar-charts.py ../results/jar/finite-210818.dat "black/finite, aalta/finite, ltl2sat/default" -p -m`
