#!/bin/bash

# VISUALIZE DATA WITH GNUPLOT
datanum=100


average_time1=`cat hdastar_1.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`
average_time2=`cat hdastar_2_analysis.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`
average_time4=`cat hdastar_4_analysis.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`
average_time8=`cat hdastar_8_analysis.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`

average_expansion1=`cat hdastar_1.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`
average_expansion2=`cat hdastar_2_analysis.dat | awk '{ sum += $5 } END { print sum/$datanum; }'`
average_expansion4=`cat hdastar_4_analysis.dat | awk '{ sum += $5 } END { print sum/$datanum; }'`
average_expansion8=`cat hdastar_8_analysis.dat | awk '{ sum += $5 } END { print sum/$datanum; }'`

average_generation1=`cat hdastar_1.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`
average_generation2=`cat hdastar_2_analysis.dat | awk '{ sum += $6 } END { print sum/$datanum; }'`
average_generation4=`cat hdastar_4_analysis.dat | awk '{ sum += $6 } END { print sum/$datanum; }'`
average_generation8=`cat hdastar_8_analysis.dat | awk '{ sum += $6 } END { print sum/$datanum; }'`


gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Wall Time"   
   set logscale y
   set terminal postscript
   set output "analysis/time.ps"
   plot "hdastar_1.dat" using (1):2 with boxplot title "1"
   replot "hdastar_2_analysis.dat" using (2):4 with boxplot title "2"
   replot "hdastar_4_analysis.dat" using (4):4 with boxplot title "4"
   replot "hdastar_8_analysis.dat" using (8):4 with boxplot title "8"
EOF

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Expanded Nodes"   
   set logscale y
   set terminal postscript
   set output "analysis/expanded.ps"
   plot "hdastar_1.dat" using (1):3 with boxplot title "1"
   replot "hdastar_2_analysis.dat" using (2):5 with boxplot title "2"
   replot "hdastar_4_analysis.dat" using (4):5 with boxplot title "4"
   replot "hdastar_8_analysis.dat" using (8):5 with boxplot title "8"
EOF

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Stored Nodes"   
   set logscale y
   set terminal postscript
   set output "analysis/stored.ps"
   plot "hdastar_1.dat" using (1):4 with boxplot title "1"
   replot "hdastar_2_analysis.dat" using (2):6 with boxplot title "2"
   replot "hdastar_4_analysis.dat" using (4):6 with boxplot title "4"
   replot "hdastar_8_analysis.dat" using (8):6 with boxplot title "8"
EOF

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average of max size of income buffers"   
   set logscale y
   set terminal postscript
   set output "analysis/income.ps"
   plot "hdastar_2_analysis.dat" using (2):2 with boxplot title "2"
   replot "hdastar_4_analysis.dat" using (4):2 with boxplot title "4"
   replot "hdastar_8_analysis.dat" using (8):2 with boxplot title "8"
EOF

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average of max size of outgo buffers"   
   set logscale y
   set terminal postscript
   set output "analysis/outgo.ps"
   plot "hdastar_2_analysis.dat" using (2):3 with boxplot title "2"
   replot "hdastar_4_analysis.dat" using (4):3 with boxplot title "4"
   replot "hdastar_8_analysis.dat" using (8):3 with boxplot title "8"
EOF
