#!/bin/bash




awk 'NR==FNR{one[$1]=$2; next;} NR==(FNR+100){two[$1]=$2;next;} NR==(FNR+200){four[$1]=$2;next;} { \
printf("%d %f %f %f %f\n", $1, one[$1], one[$1]/two[$1]/2, one[$1]/four[$1]/4, (one[$1]/$2/8)) \
}' $1 $2 $3 $4 > efficiency

gnuplot<<EOF
  set terminal pdf
  set xlabel "instance"
  set ylabel "efficiency"
  set key left
  plot "efficiency" u 1:3 w l title "2 threads"
  replot "efficiency" u 1:4 w l title "4 threads"
  set output "difficult_instances.efficiency.pdf"
  replot "efficiency" u 1:5 w l title "8 threads"
EOF

gnuplot<<EOF
  set terminal pdf
  set logscale x
  set xlabel "walltime"
  set ylabel "efficiency"
  plot "efficiency" u 2:3 w l title "2 threads"
  replot "efficiency" u 2:4 w l title "4 threads"
  set output "efficiency.pdf"
  replot "efficiency" u 2:5 w l title "8 threads"
EOF

awk 'NR==FNR{walltime[$1]=$2; one[$1]=$3; ; next;} NR==(FNR+100){two[$1]=$3;next;} NR==(FNR+200){four[$1]=$3;next;} {\
printf("%d %f %f %f %f\n", $1, walltime[$1], two[$1]/one[$1], four[$1]/one[$1], $3/one[$1])\
}' $1 $2 $3 $4 > efficiency.expd

gnuplot<<EOF
  set terminal pdf
  set logscale x
  set xlabel "walltime"
  set ylabel "search overhead"
  set yrange [,2]
  plot "efficiency.expd" u 2:3 w l title "2 threads"
  replot "efficiency.expd" u 2:4 w l title "4 threads"
  set output "search_overhead.pdf"
  replot "efficiency.expd" u 2:5 w l title "8 threads"
EOF
