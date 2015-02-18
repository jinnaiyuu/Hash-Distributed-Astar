#!/bin/bash

cat $1 | sort -n > $1.tmp
cat $2 | sort -n > $2.tmp
cat $3 | sort -n > $3.tmp

gnuplot<<EOF
  set terminal pdf
  set logscale x
  set xlabel "node expanded"
  set ylabel "bind width / node expanded"

  plot "$1.tmp" u 1:(\$2/\$1) with line title "2 threads"
  replot "$2.tmp" u 1:(\$2/\$1) with line title "4 threads"
  set output "${1}.pdf"
  replot "$3.tmp" u 1:(\$2/\$1) with line title "8 threads"

EOF

gnuplot<<EOF
  set terminal pdf
  set logscale x
  set xlabel "node expanded"
  set ylabel "bind width"

  plot "$1.tmp" u 1:2 with line title "2 threads"
  replot "$2.tmp" u 1:2 with line title "4 threads"
  set output "${1}.width.pdf"
  replot "$3.tmp" u 1:2 with line title "8 threads"

EOF
