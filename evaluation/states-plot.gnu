
datafile = "rnk-piterman-merge"
gettitle(cn) = system(sprintf("head -n1 '%s' | cut -f%d -d';'", datafile.".csv", cn))

set terminal pdf
set output datafile.".pdf"

set datafile separator ";"
unset key
set autoscale

set title "#States"

#set xrange [0:3000]
#set yrange [0:3000]

set grid xtics dashtype 3 lt 1 lw 1 lc rgb "#000000"
set grid ytics dashtype 3 lt 1 lw 1 lc rgb "#000000"

set xlabel gettitle(2)
set ylabel gettitle(3)

set arrow from 0, graph (0,0) to 7000, graph (1,1) nohead

plot \
  datafile.".csv" using 2:3 lc rgb "#127524"
