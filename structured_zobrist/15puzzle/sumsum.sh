#!/bin/bash

gnuplot<<EOF
  set terminal pdf
  set key out right
  set xrange[1:]
  set yrange[1:]
  set xlabel thread
  set ylabel speedup
  plot "sum_0" u 1:3 with lp title "Zobrist"
  replot "sum_1" u 1:3 with lp title "Structured Zobrist 2"
  replot "sum_3" u 1:3 with lp title "Structured Zobrist 4"
  replot "sum_4" u 1:3 with lp title "Structured Zobrist 8"
  replot "sum_123" u 1:3 with lp title "Abstraction"
  set output "15puzzle_spdup.pdf"
  replot x ls 1 lc rgb"black" title "linear speedup"
EOF
