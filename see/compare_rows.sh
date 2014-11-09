#!/bin/bash

column=$1
dat1=$2
dat2=$3
dat3=$4
dat4=$5
dat5=$6

echo "title = "
read title

gnuplot<<EOF
  set terminal postscript enhanced
  plot "$dat1" using 1:$column w l title "$dat1"
  replot "$dat2" using 1:$column w l title "$dat2"
  replot "$dat3" using 1:$column w l title "$dat3"
  replot "$dat4" using 1:$column w l title "$dat4"
  set output "analysis/$title.eps"
  replot "$dat5" using 1:$column w l title "$dat5"
EOF
