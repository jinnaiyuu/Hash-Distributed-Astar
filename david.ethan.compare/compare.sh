#!/bin/bash

data1=$1
data2=$2
data3=$3
data4=$4

./each.sh $data1
./each.sh $data2
./each.sh $data3
./each.sh $data4

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

gnuplot<<EOF
   set title "Korf 100 Puzzle"
   set xlabel "nodes expanded"
   set ylabel "instances solved in given expansion"
#   set terminal postscript enhanced color font ',20'
   set terminal png size 600,600 font ',15'
   set logscale x
   set key out vert
   set key center top
   plot "${data1}.correctrate.expd" u 2:1 w steps title "HDA* 1 threads (Jinnai)"
   replot "${data2}.correctrate.expd" u 2:1 w steps title "HDA* 8 threads (Jinnai)"
   replot "${data3}.correctrate.expd" u 2:1 w steps title "SafePBNF 8 threads (Ethan)"
   set output "${data1}.compare.expd.png"
   replot "${data4}.correctrate.expd" u 2:1 w steps title "HDA* 8 threads (Ethan)"
EOF
