#!/bin/bash

# VISUALIZE DATA WITH GNUPLOT
datanum=100


average_time1=`cat hdastar_1.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`
average_time2=`cat hdastar_2.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`
average_time4=`cat hdastar_4.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`
average_time8=`cat hdastar_8.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`

average_expansion1=`cat hdastar_1.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`
average_expansion2=`cat hdastar_2.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`
average_expansion4=`cat hdastar_4.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`
average_expansion8=`cat hdastar_8.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`

average_generation1=`cat hdastar_1.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`
average_generation2=`cat hdastar_2.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`
average_generation4=`cat hdastar_4.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`
average_generation8=`cat hdastar_8.dat | awk '{ sum += $4 } END { print sum/$datanum; }'`

average_income_size1=3
average_income_size2=`cat hdastar_2_analysis.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`
average_income_size4=`cat hdastar_4_analysis.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`
average_income_size8=`cat hdastar_8_analysis.dat | awk '{ sum += $2 } END { print sum/$datanum; }'`

average_outgo_size1=0
average_outgo_size2=`cat hdastar_2_analysis.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`
average_outgo_size4=`cat hdastar_4_analysis.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`
average_outgo_size8=`cat hdastar_8_analysis.dat | awk '{ sum += $3 } END { print sum/$datanum; }'`


mv result.dat hdastar

echo Threads Average_Wall_Time Average_Expansion Average_Generation Average_Income_Size Average_Outgo_Size >> result.dat
echo 1 $average_time1 $average_expansion1 $average_generation1 $average_income_size1 $average_outgo_size1 >> result.dat
echo 2 $average_time2 $average_expansion2 $average_generation2 $average_income_size2 $average_outgo_size2 >> result.dat
echo 4 $average_time4 $average_expansion4 $average_generation4 $average_income_size4 $average_outgo_size4 >> result.dat
echo 8 $average_time8 $average_expansion8 $average_generation8 $average_income_size8 $average_outgo_size8 >> result.dat

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Wall Time"   
   set terminal postscript
   set output "analysis/walltime.ps"
   plot "result.dat" using 1:2 with lines
EOF

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Node Expansion"   
   set terminal postscript
   set output "analysis/expansion.ps"
   plot "result.dat" using 1:3 with lines
EOF

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Node Generation"   
   set terminal postscript
   set output "analysis/generation.ps"
   plot "result.dat" using 1:4 with lines
EOF


