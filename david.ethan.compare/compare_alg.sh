#!/bin/bash

data1=$1
data2=$2
data3=$3
data4=$4
#data5=$5
#data6=$6
#data7=$7
#data8=$8
#data9=$9

./each.sh $data1
./each.sh $data2
./each.sh $data3
./each.sh $data4
#./each.sh $data5
#./each.sh $data6
#./each.sh $data7
#./each.sh $data8
#./each.sh $data9


gnuplot<<EOF
   set xlabel "node expanded"
   set ylabel "instances solved in given node expansion"
   set terminal pdf
#   set terminal postscript enhanced color font ',15'
#   set terminal png size 900,900 font ',15'
   set logscale x
#   set key out vert
   set key left
   plot "${data1}.correctrate.expd" u 2:1 w steps title "A*"
   replot "${data2}.correctrate.expd" u 2:1 w steps title "Parallel A*"
   replot "${data3}.correctrate.expd" u 2:1 w steps title "HDA*"
   set output "${data1}_expd.pdf"
   replot "${data4}.correctrate.expd" u 2:1 w steps title "SafePBNF"
#   replot "${data5}.correctrate.expd" u 2:1 w steps title "$data5"
#   replot "${data6}.correctrate.expd" u 2:1 w steps title "$data6"
#   replot "${data7}.correctrate.expd" u 2:1 w steps title "$data7"
#   replot "${data8}.correctrate.expd" u 2:1 w steps title "$data8"
#   replot "${data9}.correctrate.expd" u 2:1 w steps title "$data9"
EOF

echo "8, 16 threads"

gnuplot<<EOF
   set xlabel "walltime"
   set ylabel "instances solved in given time"
   set terminal pdf
#   set terminal postscript enhanced color font ',15'
#   set terminal png size 900,900 font ',15'
   set logscale x
#   set key out vert
   set key left
   plot "${data1}.correctrate" u 2:1 w steps title "HDA* 8 threads"
   replot "${data2}.correctrate" u 2:1 w steps title "HDA* 16 threads"
   replot "${data3}.correctrate" u 2:1 w steps title "SafePBNF 8 threads"
   set output "${data1}_walltime.pdf"
   replot "${data4}.correctrate" u 2:1 w steps title "SafePBNF 16 threads"
#   replot "${data5}.correctrate" u 2:1 w steps title "$data5"
#   replot "${data6}.correctrate" u 2:1 w steps title "$data6"
#   replot "${data7}.correctrate" u 2:1 w steps title "$data7"
#   replot "${data8}.correctrate" u 2:1 w steps title "$data8"
#   replot "${data9}.correctrate" u 2:1 w steps title "$data9"
EOF

exit 0

gnuplot<<EOF
   set xlabel "walltime"
   set ylabel "instances solved in given time"
   set terminal pdf
#   set terminal postscript enhanced color font ',15'
#   set terminal png size 900,900 font ',15'
   set logscale x
#   set key out vert
   set key left
   plot "${data1}.correctrate" u 2:1 w steps title "A*"
   replot "${data2}.correctrate" u 2:1 w steps title "Parallel A*"
   replot "${data3}.correctrate" u 2:1 w steps title "HDA*"
   set output "${data1}_walltime.pdf"
   replot "${data4}.correctrate" u 2:1 w steps title "SafePBNF"
#   replot "${data5}.correctrate" u 2:1 w steps title "$data5"
#   replot "${data6}.correctrate" u 2:1 w steps title "$data6"
#   replot "${data7}.correctrate" u 2:1 w steps title "$data7"
#   replot "${data8}.correctrate" u 2:1 w steps title "$data8"
#   replot "${data9}.correctrate" u 2:1 w steps title "$data9"
EOF

exit 0


gnuplot<<EOF
   set title "Korf 100 Puzzle"
   set xlabel "walltime"
   set ylabel "instances solved in given time"
#   set terminal postscript enhanced color font ',20'
   set terminal png size 600,600 font ',15'
   set logscale x
   set key out vert
   set key center bottom
   plot "${data1}.correctrate" u 2:1 w steps title "HDA* 1 threads (Jinnai)"
   replot "${data2}.correctrate" u 2:1 w steps title "HDA* 8 threads (Jinnai)"
   replot "${data3}.correctrate" u 2:1 w steps title "SafePBNF 8 threads (Ethan)"
   set output "${data1}.compare.png"
   replot "${data4}.correctrate" u 2:1 w steps title "HDA* 8 threads (Ethan)"
EOF

gnuplot<<EOF
   set terminal postscript enhanced color font ',15'
   set logscale x
   set key out vert
   set key center top
   plot "${data1}.correctrate.cum" u 2:1 w steps title "HDA* 8 threads (Jinnai)"
   replot "${data2}.correctrate.cum" u 2:1 w steps title "HDA* 1 threads (Jinnai)"
   replot "${data3}.correctrate.cum" u 2:1 w steps title "SafePBNF 8 threads (Ethan)"
   set output "${data1}.compare.cum.eps"
   replot "${data4}.correctrate.cum" u 2:1 w steps title "HDA* 8 threads (Ethan)"
EOF

