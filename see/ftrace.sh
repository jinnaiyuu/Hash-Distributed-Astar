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
# Input : "ftrace" <walltime> <thread id> <f value> <h value>
# Output: <thread id> <wall time> <f value> <h value>

awk   '/ftrace/{ if ($4 >= 10) printf("%d %f %d %d\n", $3, $2, $4, $5) }' > ftrace.dat;

# Input:  <thread number> <wall time> <f value>
# Output: <wall time> <f value>
#         for each thread
awk '($1==0){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace0.dat
awk '($1==1){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace1.dat
awk '($1==2){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace2.dat
awk '($1==3){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace3.dat
awk '($1==4){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace4.dat
awk '($1==5){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace5.dat
awk '($1==6){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace6.dat
awk '($1==7){ printf("%f %d %d\n", $2, $3, $4) }' < ftrace.dat > ftrace7.dat

# Transition plot
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace0.dat > ftrace0t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace1.dat > ftrace1t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace2.dat > ftrace2t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace3.dat > ftrace3t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace4.dat > ftrace4t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace5.dat > ftrace5t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace6.dat > ftrace6t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ftrace7.dat > ftrace7t.dat



gnuplot <<EOF
   set title "Transition of f value"
   set xlabel "wall time"
   set ylabel "f value"
   set title font "Arial, 26"
   set xlabel font "Arial, 19"
   set ylabel font "Arial, 19" offset 1,0,0
   set tics font "Arial, 20"
   set key right bottom
   set yrange [:58]
   set ytics 2

   set terminal png size 1920,480
   plot "ftrace0t.dat" using 1:2 with l title "thread 1"
   replot "ftrace1t.dat" using 1:2 with l title "thread 2"
   replot "ftrace2t.dat" using 1:2 with l title "thread 3"
   replot "ftrace3t.dat" using 1:2 with l title "thread 4"
   replot "ftrace4t.dat" using 1:2 with l title "thread 5"
   replot "ftrace5t.dat" using 1:2 with l title "thread 6"
   replot "ftrace6t.dat" using 1:2 with l title "thread 7"
   set output "analysis/ftrace.png"
   replot "ftrace7t.dat" using 1:2 with l title "thread 8"
EOF

#convert analysis/ftrace.ps analysis/ftrace.png

rm ftrace[0-3].dat
