#!/bin/bash



#instate=hdastar_distribution.dat
#outsource=hdastar_distribution_outsourcing.dat

instate=hdastar_8_dist.dat
outsource=hdastar_8_dist_os.dat


# Structure of input data
# instate:   number expdstddev gendstddev walltime  expdoverall gendoverall pathlength
# outsource: number expdstddev gendstddev walltime    expdoverall gendoverall pathlength

awk 'FNR==NR{instate[$1] = $5; next} {outsource[$1] = $5} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_expd.dat

# Compare overall expansion of nodes $5-$5

gnuplot <<EOF
   set xlabel "Outsourcing"
   set ylabel "Overall Expansion"   
   set terminal postscript
   set style data boxplot
   plot "dist_expd.dat" using (0):2
   set output "analysis/dist_expd.ps"
   replot "dist_expd.dat" using (1):3
EOF


# Compare overall generation of nodes $6-$7

awk 'FNR==NR{instate[$1] = $6; next} {outsource[$1] = $6} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_gend.dat

gnuplot <<EOF
   set xlabel "Outsourcing"
   set ylabel "Overall Generation"   
   set terminal postscript
   set style data boxplot
   plot "dist_gend.dat" using (0):2
   set output "analysis/dist_gend.ps"
   replot "dist_gend.dat" using (1):3
EOF

# Compare standard deviation of expansion of nodes $2-$2

awk 'FNR==NR{instate[$1] = $2; next} {outsource[$1] = $2} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_expd_stddev.dat

gnuplot <<EOF
   set title "Standard Deviation of Expansion of Nodes among threads with/without outsourcing"
   set xlabel "Outsourcing"
   set ylabel "Standard Deviation of Expansion of Nodes"   
   set terminal postscript
   set style data boxplot
   plot "dist_expd_stddev.dat" using (0):2
   set output "analysis/dist_expd_stddev.ps"
   replot "dist_expd_stddev.dat" using (1):3
EOF


# Compare standard deviation of generation of nodes $3-$3

awk 'FNR==NR{instate[$1] = $3; next} {outsource[$1] = $3} \
END { \
for (i = 1; i <=100; i++) \
  if ((length(instate[i]) != 0) && (length(outsource[i]) != 0)) \
    printf("%s %s %s\n", i, instate[i], outsource[i]) \
}' ${instate} ${outsource} > dist_gend_stddev.dat

gnuplot <<EOF
   set title "Standard Deviation of Generation of Nodes among threads with/without outsourcing"
   set xlabel "Outsourcing"
   set ylabel "Standard Deviation of Generation of Nodes"   
   set terminal postscript
   set style data boxplot
   plot "dist_gend_stddev.dat" using (0):2
   set output "analysis/dist_gend_stddev.ps"
   replot "dist_gend_stddev.dat" using (1):3
EOF


