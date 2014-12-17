#!/bin/bash

awk 'NR==FNR{one[$1]=$2; ; next;} NR==(FNR+100){two[$1]=$2;next;} NR==(FNR+200){four[$1]=$2;next;} {\
printf("%d %f %f %f %f\n", $1, one[$1], one[$1]/two[$1]/2, one[$1]/four[$1]/4, one[$1]/$2/8)\
}' hdastar.difficult_instances.1threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1108214642.o79 hdastar.difficult_instances.2threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1108214922.o80 hdastar.difficult_instances.4threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1108174717.o76 hdastar.difficult_instances.8threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1108174649.o75 > efficiency

gnuplot<<EOF
  set terminal png
  set xlabel "instance"
  set ylabel "efficiency"
  plot "efficiency" u 1:3 w l title "2 threads"
  replot "efficiency" u 1:4 w l title "4 threads"
  set output "difficult_instances.efficiency.png"
  replot "efficiency" u 1:5 w l title "8 threads"
EOF

gnuplot<<EOF
  set terminal png
  set logscale x
  set xlabel "walltime"
  set ylabel "efficiency"
  plot "efficiency" u 2:3 w l title "2 threads"
  replot "efficiency" u 2:4 w l title "4 threads"
  set output "difficult_instances.efficiency.walltime.png"
  replot "efficiency" u 2:5 w l title "8 threads"
EOF
