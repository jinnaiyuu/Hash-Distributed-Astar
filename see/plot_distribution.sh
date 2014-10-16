#!/bin/bash



#instate=hdastar_distribution.dat
#outsource=hdastar_distribution_outsourcing.dat

instate=hdastar_8_dist.dat
outsource=hdastar_8_dist_os2.dat


# Compare walltime $4-$5

awk 'FNR==NR{instate[$1] = $4; next} {outsource[$1] = $5} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > walltime_os.dat

gnuplot <<EOF
   set title "Walltime Comparison"
   set xlabel "HDA*"
   set ylabel "JOHDA*"
   set title font "Arial, 26"
   set xlabel font "Arial, 26"
   set ylabel font "Arial, 26"
   set tics font "Arial, 20"

   set size ratio 1
   set view equal xy
   set terminal postscript
   plot "walltime_os.dat" using 2:3 title "walltime"
   set output "analysis/walltime_os.ps"
   replot x with line ls 1 title "Balance"
EOF


# Structure of input data
# instate:   number expdstddev gendstddev walltime  expdoverall gendoverall pathlength
# outsource: number expdstddev gendstddev (outsourcing) walltime    expdoverall gendoverall pathlength

awk 'FNR==NR{instate[$1] = $5; next} {outsource[$1] = $6} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_expd.dat

# Compare overall expansion of nodes $5-$5

gnuplot <<EOF
   set terminal postscript
   set title "Node Expansion Comparison"
   set xlabel "HDA*"
   set ylabel "JOHDA*"   
   set title font "Arial, 26"
   set xlabel font "Arial, 26"
   set ylabel font "Arial, 26"
   set tics font "Arial, 20"
   plot "dist_expd.dat" using 2:3 title "Expansion"
   set output "analysis/dist_expd.ps"
   replot x with line ls 1 title "Balance"
EOF



# Compare overall generation of nodes $6-$7

awk 'FNR==NR{instate[$1] = $6; next} {outsource[$1] = $7} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_gend.dat

gnuplot <<EOF
   set title "Overall Node Generation (8 threads)"
   set xlabel "HDA*"
   set ylabel "JOHDA*"   
   set terminal postscript
   plot "dist_gend.dat" using 2:3
   set output "analysis/dist_gend.ps"
   replot x with line ls 1
EOF

# Compare standard deviation of expansion of nodes $2-$2

awk 'FNR==NR{instate[$1] = $2; next} {outsource[$1] = $2} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_expd_stddev.dat

gnuplot <<EOF
   set title "Standard Deviation of Node Expansion (8 threads)"
   set xlabel "HDA*"
   set ylabel "JOHDA*"   
   set terminal postscript
   plot "dist_expd_stddev.dat" using 2:3
   set output "analysis/dist_expd_stddev.ps"
   replot x with line ls 1
EOF


# Compare standard deviation of generation of nodes $3-$3

awk 'FNR==NR{instate[$1] = $3; next} {outsource[$1] = $3} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_gend_stddev.dat

gnuplot <<EOF
   set title "Standard Deviation of Node Generation (8 threads)"
   set xlabel "HDA*"
   set ylabel "JOHDA*"   
   set terminal postscript
   set size ratio -1
   plot "dist_gend_stddev.dat" using 2:3
   set output "analysis/dist_gend_stddev.ps"
   replot x with line ls 1
EOF





# Convert them to png so that easy to convey for presentations.

#convert analysis/*.ps analysis/*.png
