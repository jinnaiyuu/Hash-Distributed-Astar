#!/bin/bash


# Plots the Speedup of the HDA*

date=`date +"%m%d"`

astar=astar.dat
data1=hdastar_1_1013_noos_164836.dat
data2=hdastar_2_1014_noos_164838.dat
data4=hdastar_4_1013_noos_164837.dat
data8=hdastar_8_1012_noos_163132.dat
# Input Data Structure
# <problem number> <walltime> <expand> <generate> <path length>




# Search Overhead and Speedup

# Input raw data file and print search overhead and speedup ratio.
awk 'FNR==NR{astar_wall[$1]=$2; astar_expd[$1]=$3; next} { \
if (length(astar_wall[$1]) != 0) \
if (length($2) != 0) \
if ($1 <= 10) \
if (astar_wall[$1]/$2 <= 8.3) \
print astar_wall[$1]/$2, $3/astar_expd[$1]}' $astar $data4 > searchoverhead_speedup_astar_4threads_$date.dat

gnuplot <<EOF
   set terminal postscript color
   set title "Search Overhead and Speedup of Wall Time"
   set title font "Arial, 20"
   set xlabel "nodes expanded relative to A*"
   set xlabel font "Arial, 20"
   set ylabel "Wall time relative to A*"
   set ylabel font "Arial, 20"
   set nokey
   set output "analysis/searchoverhead_speedup_astar_4threads_$date.ps"
   plot "searchoverhead_speedup_astar_4threads_$date.dat" using 1:2
EOF

exit 0

awk 'FNR==NR{astar[$1]=$2; next} (FNR+100)==NR{a[$1]=$2; next} (FNR+200)==NR{b[$1]=$2; next} (FNR+300)==NR{c[$1]=$2; next} \
(FNR+400)==NR{\
if (length(astar[$1]) != 0) \
if (length(a[$1]) != 0) \
if (length(b[$1]) != 0) \
if (length(c[$1]) != 0) \
if (length($2) != 0) \
print astar[$1], a[$1], b[$1], c[$1], $2}' ${astar} ${data1} ${data2} ${data4} ${data8} > speedup_$date.dat


# Two awk scripts can be piped. For debugging purpose, we save the intermediate data structure.
#awk 'FNR==NR{astar[$1]=$2; next} (FNR+100)==NR{print astar[$1], $4}' astar.dat hdastar_1_opt.dat > speedup.dat
awk '{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd


awk 'FNR==NR{astar[$1]=$2; next} (FNR+100)==NR{a[$1]=$2; next} (FNR+200)==NR{b[$1]=$2; next} (FNR+300)==NR{c[$1]=$2; next} \
(FNR+400)==NR{\
if (length(astar[$1]) != 0) \
if (length(a[$1]) != 0) \
if (length(b[$1]) != 0) \
if (length(c[$1]) != 0) \
if (length($2) != 0) \
print astar[$1], a[$1], b[$1], c[$1], $2}' ${astar} ${data1} ${data2} ${data4} ${data8} > speedup_$date.dat

awk '{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd

gnuplot <<EOF
   set terminal postscript color
   set title "Speedup of HDA* Compared to A*"
   set title font "Arial, 20"
   set xlabel "Number of cores"
   set xlabel font "Arial, 20"
   set ylabel "Speedup"   
   set ylabel font "Arial, 20"
   plot "spd" using 1:2 with lp title "Speedup of HDA*"
   set output "analysis/speedup_$date.ps"
   replot x with line ls 1 linecolor "black" title "Perfect Speedup"
EOF


# Speed up of top 10 problems
awk 'NR<=10{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd10
gnuplot <<EOF
   set terminal postscript color
   set title "Speedup of HDA* Compared to A*"
   set title font "Arial, 26"
   set xlabel "Number of cores"
   set xlabel font "Arial, 26"
   set ylabel "Speedup"   
   set ylabel font "Arial, 26"
   set tics font "Arial, 20"
   plot "spd10" using 1:2 with lp title "Speedup of HDA*"
   set output "analysis/speedup_top10_$date.ps"
   replot x with line ls 1 linecolor "black" title "Perfect Speedup"
EOF

# Speed up of top 12 problems
awk 'NR<=12{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd12
gnuplot <<EOF
   set title "Speedup of HDA* Compared to A*"
   set xlabel "Number of cores"
   set ylabel "Speedup"   
   set terminal postscript color
   plot "spd12" using 1:2 with lp
   set output "analysis/speedup_top12_$date.ps"
   replot x with line ls 1
EOF


# Speed up of top 15 problems
awk 'NR<=15{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd15
gnuplot <<EOF
   set terminal postscript color
   set title "Speedup of HDA* Compared to A*"
   set title font "Arial, 26"
   set xlabel "Number of cores"
   set xlabel font "Arial, 26"
   set ylabel "Speedup"   
   set ylabel font "Arial, 26"
   set tics font "Arial, 20"
   plot "spd15" using 1:2 with lp title "Speedup of HDA*"
   set output "analysis/speedup_top15_$date.ps"
   replot x with line ls 1 linecolor "black" title "Perfect Speedup"
EOF

# Speed up of top 20 problems
awk 'NR<=20{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd20
gnuplot <<EOF
   set title "Speedup of HDA* Compared to A*"
   set xlabel "Number of cores"
   set ylabel "Speedup"   
   set terminal postscript color
   plot "spd20" using 1:2 with lp
   set output "analysis/speedup_top20_$date.ps"
   replot x with line ls 1
EOF


# Speed up of top 30 problems
awk 'NR<=30{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd30
gnuplot <<EOF
   set title "Speedup of HDA* Compared to A*"
   set xlabel "Number of cores"
   set ylabel "Speedup"   
   set terminal postscript color
   plot "spd30" using 1:2 with lp
   set output "analysis/speedup_top30_$date.ps"
   replot x with line ls 1
EOF

# Speed up of bot 70 problems
awk 'NR>=30{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup_$date.dat > spd70
gnuplot <<EOF
   set title "Speedup of HDA* Compared to A*"
   set xlabel "Number of cores"
   set ylabel "Speedup"   
   set terminal postscript color
   plot "spd30" using 1:2 with lp
   set output "analysis/speedup_bot70_$date.ps"
   replot x with line ls 1
EOF


# Plots the node expansion & genration compared to astar for each threads.

awk 'FNR==NR{astar[$1]=$3; next} (FNR+100)==NR{a[$1]=$3; next} (FNR+200)==NR{b[$1]=$3; next} (FNR+300)==NR{c[$1]=$3; next} \
(FNR+400)==NR{\
if (length(astar[$1]) != 0) \
if (length(a[$1]) != 0) \
if (length(b[$1]) != 0) \
if (length(c[$1]) != 0) \
if (length($3) != 0) \
print astar[$1], a[$1], b[$1], c[$1], $3}' ${astar} ${data1} ${data2} ${data4} ${data8} > expd_$date.dat

awk '{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("0 %f\n1 %f\n2 %f\n4 %f\n8 %f", astar, single, duo, quadra, oct)}' \
expd_$date.dat > expd

gnuplot <<EOF
   set title "Search Overhead"
   set xlabel "Number of cores"
   set ylabel "Average Expansion"   
   set terminal postscript
   set output "analysis/expansion_$date.ps"
   plot "expd" using 1:2 with lp
EOF

# Plots the node expansion & genration compared to astar for each threads.

awk '{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", single/astar, duo/astar, quadra/astar, oct/astar)}' \
expd_$date.dat > expd

gnuplot <<EOF
   set terminal postscript color
   set title "Search Overhead"
   set title font "Arial, 26"
   set xlabel "Number of cores"
   set xlabel font "Arial, 26"
   set ylabel "Ratio"   
   set ylabel font "Arial, 26"
   set tics font "Arial, 20"
   set nokey
   set output "analysis/expansion_ratio_$date.ps"
   plot "expd" using 1:2 with lp title "The Ratio of Node Expansion Compared to A*"
EOF


awk 'FNR==NR{astar[$1]=$4} (FNR+100)==NR{a[$1]=$4; next} (FNR+200)==NR{b[$1]=$4; next} (FNR+300)==NR{c[$1]=$4; next} \
{print astar[$1]/a[$1], astar[$1]/b[$1], astar[$1]/c[$1], astar[$1]/$4}' ${astar} ${data1} ${data2} ${data4} ${data8} > generation_$date.dat

awk 'BEGIN{datanum=100.0} /inf/{--datanum; next} /nan/{--datanum; next} \
{ duo+=$1; quadra+=$2; oct+=$3; } END{ printf("2 %f\n4 %f\n8 %f", duo/datanum, quadra/datanum, oct/datanum)}' generation_$date.dat > gend

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Generation Ratio (Compared to 1 thread)"   
   set terminal postscript
   set output "analysis/generation_$date.ps"
   plot "gend" using 1:2 with lp
EOF

exit 0
# $2 = income buffer size
# $3 = outgo buffer size

gnuplot <<EOF
   set title "Maximum Size of Income Buffer"
   set xlabel "Number of Cores"
   set ylabel "income buffer size"   
   set xrange [0:10]
   set terminal postscript
   plot "$data1" using (1):2 pt 2
   replot "$data2" using (2):2 pt 2
   replot "$data4" using (4):2 pt 2
   set output "analysis/income_$date.ps"
   replot "$data8" using (8):2 pt 2
EOF

gnuplot <<EOF
   set title "Maximum Size of Outgo Buffer"
   set xlabel "Number of Cores"
   set ylabel "outgo buffer size"   
   set xrange [0:10]
   set terminal postscript
   plot "$data1" using (1):3 pt 2
   replot "$data2" using (2):3 pt 2
   replot "$data4" using (4):3 pt 2
   set output "analysis/outgo_$date.ps"
   replot "$data8" using (8):3 pt 2
EOF


#rm spd expd gene
