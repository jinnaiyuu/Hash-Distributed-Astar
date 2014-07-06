#!/bin/bash

# VISUALIZE DATA WITH GNUPLOT

./see/gnuplot <<EOF
   set ylabel "Wall Time"   
   set logscale y
   set terminal postscript
   set output "$1.ps"
   plot "$1.dat" using (0):2 with boxplot
   
EOF

