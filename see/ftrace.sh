#!/bin/bash

usage(){
    cat<<EOF
ftrace is a command for analyzing the behavior of f value for hdastar.
The input data is hdastar.o<job id>.
It parse down all ftrace output from the text.
Then plot the movements of f values for each thread.


Input data format:
  ftrace data is in a format like below.
  ftrace <thread number> <f value> <wall time>

Output text data format:
  Wall time comes second as it is the X-axis in plot.

  <thread number> <wall time> <f value>

Usage:
  ftrace.sh << Input data file

TODO:
  So far it only accepts 4 thread HDAstar.
  Should refactor it for any number of threads.

EOF
}

#tnum=`awk '/thread number/{print $4}'`

# If the line contains "ftrace", read the second, third and forth paraemter.
# Output: <thread number> <wall time> <f value>

#awk '/initial heuristic/{printf("%d %f %d\n", 1, 0, $4)}'
#awk '/initial heuristic/{printf("%d %f %d\n", 2, 0, $4)}'
#awk '/initial heuristic/{printf("%d %f %d\n", 3, 0, $4)}'

#awk '/initial heuristic/{printf("%d %f %d\n", 0, 0, $4)} \
#    /ftrace/{ printf("%d %f %d\n", $2, $4, $3) }' > ftrace.dat;

awk   '/ftrace/{ printf("%d %f %d\n", $2, $4, $3) }' > ftrace.dat;

# Input:  <thread number> <wall time> <f value>
# Output: <wall time> <f value>
#         for each thread
awk '($1==0){ printf("%f %d\n", $2, $3) }' < ftrace.dat > ftrace0.dat
awk '($1==1){ printf("%f %d\n", $2, $3) }' < ftrace.dat > ftrace1.dat
awk '($1==2){ printf("%f %d\n", $2, $3) }' < ftrace.dat > ftrace2.dat
awk '($1==3){ printf("%f %d\n", $2, $3) }' < ftrace.dat > ftrace3.dat


gnuplot <<EOF
   set ylabel "Trace of f value for each thread"   
   set terminal png size 1280, 960
   set ytics 2
   plot "ftrace0.dat" using 1:2 with lp
   replot "ftrace1.dat" using 1:2 with lp
   replot "ftrace2.dat" using 1:2 with lp
   set output "analysis/ftrace.png"
   replot "ftrace3.dat" using 1:2 with lp
EOF

rm ftrace[0-3].dat
