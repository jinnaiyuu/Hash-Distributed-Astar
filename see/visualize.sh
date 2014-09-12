#!/bin/bash

# VISUALIZE DATA WITH GNUPLOT

datnum=`wc -l $1.dat | awk '{print $1}'`

datastar=`wc -l astar.dat | awk '{print $1}'`

gnuplot <<EOF
   set xlabel "$datnum instances solved"
   set ylabel "Wall Time"   
   set logscale y
   set terminal postscript
   set output "$1.ps"
   plot "$1.dat" using (0):2 with boxplot title "$1 solved $datnum"
EOF

gnuplot <<EOF
   set ylabel "Wall Time"   
   set logscale y
   set terminal postscript
   plot "astar.dat" using (0):2 with boxplot title "astar solved $datastar"
   set output "15puzzle.ps"
   replot "$1.dat" using (1):2 with boxplot title "$1 solved $datnum"
EOF
