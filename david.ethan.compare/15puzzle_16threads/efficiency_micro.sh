#!/bin/bash

fs1=`wc -l $1 | awk '{print $1'}`
fs16=`wc -l $2 | awk '{print $1'}`
fs24=`wc -l $3 | awk '{print $1'}`

echo $fs1 $fs16 $fs24
fs=$fs1

awk -v fs=$fs '\
NR==FNR{one[$1]=$2; next;} \
NR==(FNR+fs){sixteen[$1]=$2;next;} \
{ \
printf("%d %f %f %f \n", $1, one[$1], one[$1]/sixteen[$1], one[$1]/$2)\
}' $1 $2 $3 | sort -n -k 2 | head -n `expr $fs - 1` > speedup

gnuplot<<EOF
  set terminal pdf
  set logscale x
  set logscale y
  set xlabel "walltime"
  set ylabel "speedup"
  set key left
  plot "speedup" u 2:3 w l title "2 threads"
  replot "speedup" u 2:4 w l title "16 threads"
  set output "speedup_24puzzle_micro.pdf"
  replot "speedup" u 2:5 w l title "24 threads"

EOF


cat speedup | awk -v fs=$fs '{\
spdup16+=$3; spdup24+=$4; } \
END{\
printf("1 1\n"); \
printf("16 %f\n", spdup16/fs); \
printf("24 %f\n", spdup24/fs); \
}' > sumspeedup

gnuplot<<EOF
  set terminal pdf
  set xlabel "threads"
  set ylabel "speedup"
  set key outside below
  plot "sumspeedup" u 1:2 title "speedup" with lp ls 1
  set output "speedup.pdf"
  replot x title "Perfect speedup"
EOF

exit 0
