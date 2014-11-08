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
  ftrace.sh <Input data file>

TODO:
  So far it only accepts 4 thread HDAstar.
  Should refactor it for any number of threads.

EOF
}

filename=$1

#tnum=`awk '/thread number/{print $4}'`

# If the line contains "ftrace", read the second, third and forth paraemter.
# Input : "ftrace" <walltime> <thread id> <f value> <h value>
# Output: <thread id> <wall time> <f value> <h value>

echo "start parsing... ${filename}"
awk '/ftrace/{ if ($4 >= 10) printf("%d %f %d %d\n", $3, $2, $4, $5) }' ${filename} > ${filename}ftrace;


echo "Parsed to ftrace.dat"
# Input:  <thread number> <wall time> <f value>
# Output: <wall time> <f value>
#         for each thread
awk '($1==0){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace0.dat
awk '($1==1){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace1.dat
awk '($1==2){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace2.dat
awk '($1==3){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace3.dat
awk '($1==4){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace4.dat
awk '($1==5){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace5.dat
awk '($1==6){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace6.dat
awk '($1==7){ printf("%f %d %d\n", $2, $3, $4) }' < ${filename}ftrace > ${filename}ftrace7.dat
echo "Parsed for each thread"

# Transition plot
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace0.dat > ${filename}ftrace0t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace1.dat > ${filename}ftrace1t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace2.dat > ${filename}ftrace2t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace3.dat > ${filename}ftrace3t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace4.dat > ${filename}ftrace4t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace5.dat > ${filename}ftrace5t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace6.dat > ${filename}ftrace6t.dat
awk 'NR==1 {f=$2; h=$3} NR>1{ printf("%f %d %d\n%f %d %d\n", $1-0.000001, f, h, $1, $2, $3); f=$2; h=$3}' < ${filename}ftrace7.dat > ${filename}ftrace7t.dat
echo "Inserted transition"


### f & htrace

goalf=`awk '$2=="solution"{printf("%d", $4-1)}' ${filename}`

gnuplot <<EOF
   set title "Transition of f & h value"
   set xlabel "wall time"
   set ylabel "f & h value"
   set title font "Arial, 26"
   set xlabel font "Arial, 19"
   set ylabel font "Arial, 19" offset 1,0,0
   set tics font "Arial, 20"
   set key right bottom

   set terminal png size 1920,960
   plot "${filename}ftrace0t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 1"
   replot "${filename}ftrace1t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 2"
   replot "${filename}ftrace2t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 3"
   replot "${filename}ftrace3t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 4"
   replot "${filename}ftrace4t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 5"
   replot "${filename}ftrace5t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 6"
   replot "${filename}ftrace6t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 7"
   set output "analysis/${filename}fhtrace.png"
   replot "${filename}ftrace7t.dat" using 1:(\$2==$goalf ? \$3 : 1/0) with l title "thread 8"
EOF

echo "htrace done"

exit 0

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

   set terminal png size 3840,480
   plot "${filename}ftrace0t.dat" using 1:2 with l title "thread 1"
   replot "${filename}ftrace1t.dat" using 1:2 with l title "thread 2"
   replot "${filename}ftrace2t.dat" using 1:2 with l title "thread 3"
   replot "${filename}ftrace3t.dat" using 1:2 with l title "thread 4"
   replot "${filename}ftrace4t.dat" using 1:2 with l title "thread 5"
   replot "${filename}ftrace5t.dat" using 1:2 with l title "thread 6"
   replot "${filename}ftrace6t.dat" using 1:2 with l title "thread 7"
   set output "analysis/${filename}.ftrace.png"
   replot "${filename}ftrace7t.dat" using 1:2 with l title "thread 8"
EOF

echo "done!"

scp analysis/${filename}.ftrace.png yuu@yuu:/home/yuu/workspace/15PuzzleOpt/see/analysis/ftrace

#convert analysis/ftrace.ps analysis/ftrace.png




rm ${filename}ftrace
rm ${filename}ftrace[0-7].dat
rm ${filename}ftrace[0-7]t.dat
