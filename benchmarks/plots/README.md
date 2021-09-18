SCATTER PLOT
------------
$ python3 make-scatter-plot.py ../results/jar/finite-210811.dat aalta/finite black/finite 400 --pdf --html --log

$ python3 make-scatter-plot.py ../results/jar/future-210817.dat aalta/v2 black/mathsat 400 --pdf --html --log


SURVIVAL PLOT
-------------
$ python3 make-survival-plot.py ../results/jar/finite-210811.dat "aalta/finite, black/finite" 360 -p -m -t 1 -i 100

$ python3 make-survival-plot.py ../results/jar/future-210817.dat "aalta/v2, black/mathsat" 360 -p -m -t 360 -i 10000
