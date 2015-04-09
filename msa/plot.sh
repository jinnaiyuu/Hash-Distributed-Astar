#!/bin/bash

gnuplot<<EOF
set terminal pdf
set xlabel "threads"
set ylabel "speedup"

plot 'hdastar' w l
set output "msa_speedup.pdf"
replot 'safepbnf' w l
EOF
