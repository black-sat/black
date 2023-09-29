# Plots

How to use the scripts to generate the plots from raw data.

You can see the meaning of each command line argument with, for example, `python3 make-survival-plot.py -h`.

## Scatter plots

These are the commands used to generate the plots in the paper. The commands are repeated for each tool that we compared with BLACK.

`$ python3 make-scatter-plot.py ../results/jar/future-210817.dat aalta/v2 black/z3 400 --pdf --html --log`

`$ python3 make-scatter-plot.py ../results/jar/past-210818.dat nuXmv/sbmc black/z3 400 --pdf --html --log`

`$ python3 make-scatter-plot.py ../results/jar/finite-230927.dat aalta/finite black/finite 400 --pdf --html --log`

## Cactus plots

These are the commands used to generate the plots in the paper.

`$ python3 make-cactus-plot.py ../results/jar/merged_ls4-230928_into_future-230928.csv "black/z3, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph, ls4/default" -p -m -i 10000 --both`

`$ python3 make-cactus-plot.py ../results/jar/past-221008.dat "black/z3, nuXmv/sbmc, nuXmv/klive" -p -m -i 10000 --both`

`$ python3 make-cactus-plot.py ../results/jar/finite-230927.dat "black/finite, aalta/finite, ltl2sat/default" -p -m -i 10000 --both`

One can also filter the survival plots by family with, for example, the following command.

`$ python3 make-survival-plot.py ../results/jar/future-210817.dat "black/mathsat, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph" -p -m -i 10000 --both --family acacia`

## Bar charts

These are the commands used to generate the plots in the paper.

`python3 make-bar-charts.py ../results/jar/merged_ls4-230928_into_future-230928.csv "black/z3, nuXmv/sbmc, nuXmv/klive, aalta/v2, leviathan/default, pltl/tree, pltl/graph, ls4/default" -p -m`

`python3 make-bar-charts.py ../results/jar/past-221008.dat "black/z3, nuXmv/sbmc, nuXmv/klive" -p -m -w 500`

`python3 make-bar-charts.py ../results/jar/finite-230927.dat "black/finite, aalta/finite, ltl2sat/default" -p -m`
