#!/bin/bash

fs=`wc -l $1 | awk '{print $1'}`

awk -v fs=$fs '\
NR==FNR{one[$1]=$2; next;} \
NR==(FNR+fs){two[$1]=$2;next;} \
NR==(FNR+fs*2){four[$1]=$2;next;} \
NR==(FNR+fs*3){eight[$1]=$2;next;} \
NR==(FNR+fs*4){twelve[$1]=$2;next;} \
NR==(FNR+fs*5){sixteen[$1]=$2;next;} \
{ \
printf("%d %f %f %f %f %f %f %f\n", $1, one[$1], one[$1]/two[$1]/2, one[$1]/four[$1]/4, one[$1]/eight[$1]/8, one[$1]/twelve[$1]/12, one[$1]/sixteen[$1]/16, (one[$1]/$2/24)) \
}' $1 $2 $3 $4 $5 $6 $7 | sort -n -k 2 > efficiency

#sort -n -t 2 efficiency > eff_sort

gnuplot<<EOF
  set terminal pdf
  set logscale x
  set xlabel "walltime"
  set ylabel "efficiency"
  set key left
  plot "efficiency" u 2:3 w l title "2 threads"
  replot "efficiency" u 2:4 w l title "4 threads"
  replot "efficiency" u 2:5 w l title "8 threads"
  replot "efficiency" u 2:6 w l title "12 threads"
  replot "efficiency" u 2:7 w l title "16 threads"
  set output "efficiency_24puzzle_micro.pdf"
  replot "efficiency" u 2:8 w l title "24 threads"
EOF

awk -v fs=$fs '\
NR==FNR{walltime[$1]=$2; one[$1]=$3; next;} \
NR==(FNR+fs){two[$1]=$3;next;} \
NR==(FNR+fs*2){four[$1]=$3;next;} \
NR==(FNR+fs*3){eight[$1]=$3;next;} \
NR==(FNR+fs*4){twelve[$1]=$3;next;} \
NR==(FNR+fs*5){sixteen[$1]=$3;next;} \
{\
printf("%d %f %f %f %f %f %f %f\n", $1, walltime[$1], two[$1]/one[$1], four[$1]/one[$1], eight[$1]/one[$1], twelve[$1]/one[$1], sixteen[$1]/one[$1], $3/one[$1])\
}' $1 $2 $3 $4 $5 $6 $7 | sort -n -k 2 > efficiency.expd

gnuplot<<EOF
  set terminal pdf
  set logscale x
  set xlabel "walltime"
  set ylabel "search overhead"
  plot "efficiency.expd" u 2:3 w l title "2 threads"
  replot "efficiency.expd" u 2:4 w l title "4 threads"
  replot "efficiency.expd" u 2:5 w l title "8 threads"
  replot "efficiency.expd" u 2:6 w l title "12 threads"
  replot "efficiency.expd" u 2:7 w l title "16 threads"
  set output "search_overhead_24puzzle_micro.pdf"
  replot "efficiency.expd" u 2:8 w l title "24 threads"
EOF

cp *_micro.pdf ../../paper/eps

exit 0

gnuplot<<EOF
  set terminal pdf
  set xlabel "instance"
  set ylabel "efficiency"
  plot "efficiency" u 1:3 w l title "2 threads"
  replot "efficiency" u 1:4 w l title "4 threads"
  replot "efficiency" u 1:5 w l title "8 threads"
  replot "efficiency" u 1:6 w l title "16 threads"
  set output "difficult_instances.efficiency.pdf"
  replot "efficiency" u 1:7 w l title "24 threads"
EOF

