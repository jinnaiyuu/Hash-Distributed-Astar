#!/bin/bash

xfile=$1
xrow=$2
yfile=$3
xrownum=`cat $xfile | awk 'BEGIN{nf = 0} {if(nf < NF) nf = NF} END {print NF}'`
yrow=`expr $4 + $xrownum`

read -p "Enter Graph Title: " title

paste $xfile $yfile > xy_data



gnuplot <<EOF
   set terminal postscript eps
   set title "$title"
   set xlabel "${xfile}-${xrow}"
   set ylabel "${yfile}-$4"
   fit a*x+b "xy_data" using $xrow:$yrow via a,b
   ti = sprintf("%f", a)
   plot x with line ls 1
   replot "xy_data" using $xrow:$yrow 
   set output "analysis/$title.eps"
   replot a*x+b title ti
EOF

rm xy_data
