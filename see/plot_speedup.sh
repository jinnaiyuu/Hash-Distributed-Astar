#!/bin/bash


# Plots the Speedup of the HDA*

astar=astar.dat
data1=hdastar_1_opt.dat
data2=hdastar_2_opt.dat
data4=hdastar_4_opt.dat
data8=hdastar_8_opt.dat

# Two awk scripts can be piped. For debugging purpose, we save the intermediate data structure.
#awk 'FNR==NR{astar[$1]=$2; next} (FNR+100)==NR{print astar[$1], $4}' astar.dat hdastar_1_opt.dat > speedup.dat


awk 'FNR==NR{astar[$1]=$2; next} (FNR+100)==NR{a[$1]=$4; next} (FNR+200)==NR{b[$1]=$4; next} (FNR+300)==NR{c[$1]=$4; next} \
(FNR+400)==NR{\
if (length(astar[$1]) != 0) \
if (length(a[$1]) != 0) \
if (length(b[$1]) != 0) \
if (length(c[$1]) != 0) \
if (length($4) != 0) \
print astar[$1], a[$1], b[$1], c[$1], $4}' ${astar} ${data1} ${data2} ${data4} ${data8} > speedup.dat

awk '{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", astar/single, astar/duo, astar/quadra, astar/oct)}' \
speedup.dat > spd

gnuplot <<EOF
   set title "Speedup of HDA* Compared to A*"
   set xlabel "Number of cores"
   set ylabel "Speedup"   
   set terminal postscript color
   plot "spd" using 1:2 with lp
   set output "analysis/speedup_of_hdastar_compared_to_astar.ps"
   replot x with line ls 1
EOF

# Speedup over single core HDA*
awk '{ single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", 1, single/duo, single/quadra, single/oct)}' \
speedup.dat > spd

gnuplot <<EOF
   set title "Speedup of HDA* Compared to single thread HDA*"
   set xlabel "Number of cores"
   set ylabel "Speedup"   
   set terminal postscript color
   plot "spd" using 1:2 with lp
   set output "analysis/speedup-compared-to-single-core-hdastar.ps"
   replot x with line ls 1
EOF

# Plots the node expansion & genration compared to astar for each threads.

awk 'FNR==NR{astar[$1]=$3; next} (FNR+100)==NR{a[$1]=$5; next} (FNR+200)==NR{b[$1]=$5; next} (FNR+300)==NR{c[$1]=$5; next} \
(FNR+400)==NR{\
if (length(astar[$1]) != 0) \
if (length(a[$1]) != 0) \
if (length(b[$1]) != 0) \
if (length(c[$1]) != 0) \
if (length($4) != 0) \
print astar[$1], a[$1], b[$1], c[$1], $5}' ${astar} ${data1} ${data2} ${data4} ${data8} > expd.dat

awk '{ astar += $1; single+= $2; duo+=$3; quadra+=$4; oct+=$5; } END{ printf("1 %f\n2 %f\n4 %f\n8 %f", single/astar, duo/astar, quadra/astar, oct/astar)}' \
expd.dat > expd

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Expansion (Compared to astar)"   
   set terminal postscript
   set output "analysis/expansion.ps"
   plot "expd" using 1:2 with lp
EOF


awk 'FNR==NR{astar[$1]=$4} (FNR+100)==NR{a[$1]=$6; next} (FNR+200)==NR{b[$1]=$6; next} (FNR+300)==NR{c[$1]=$6; next} \
{print astar[$1]/a[$1], astar[$1]/b[$1], astar[$1]/c[$1], astar[$1]/$6}' ${astar} ${data1} ${data2} ${data4} ${data8} > gend_overhead.dat

awk 'BEGIN{datanum=100.0} /inf/{datanum--; next} /nan/{datanum--; next} \
{ duo+=$1; quadra+=$2; oct+=$3; } END{ printf("2 %f\n4 %f\n8 %f", duo/datanum, quadra/datanum, oct/datanum)}' gene_overhead.dat > gene

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Generation Ratio (Compared to 1 thread)"   
   set terminal postscript
   set output "analysis/generation.ps"
   plot "gene" using 1:2 with lp
EOF


# Analysis of other factors

data1=hdastar_1.dat
data2=hdastar_2.dat
data4=hdastar_4.dat
data8=hdastar_8.dat


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
   set output "analysis/income.ps"
   replot "$data8" using (8):2 pt 2
EOF

gnuplot <<EOF
   set terminal postscript
   set title "Maximum Size of Local Buffer"
   set xlabel "Number of Cores"
   set ylabel "buffer size"

   set title font "Arial, 26"
   set xlabel font "Arial, 26"
   set ylabel font "Arial, 26"
   set tics font "Arial, 20"
   set xrange [0:10]

   plot "$data1" using (1):3 pt 2 title "1 thread"
   replot "$data2" using (2):3 pt 2 title "2 threads"
   replot "$data4" using (4):3 pt 2 title "4 threads"
   set output "analysis/outgo.ps" 
   replot "$data8" using (8):3 pt 2 title "8 threads"
EOF


# Outgo buffer size <-> Searchoverhead 

awk 'FNR==NR{astar[$1]=$3; next} \
(FNR+100)==NR{ss[$1]=$5; sb[$1]=$3; next} \
(FNR+200)==NR{ds[$1]=$5; db[$1]=$3; next} \
(FNR+300)==NR{qs[$1]=$5; qb[$1]=$3; next} \
(FNR+400)==NR{\
if (length(astar[$1]) != 0) \
if (length(ss[$1]) != 0) \
if (length(ds[$1]) != 0) \
if (length(qs[$1]) != 0) \
if (length($5) != 0) \
print ss[$1]/astar[$1], sb[$1], ds[$1]/astar[$1], db[$1], \
qs[$1]/astar[$1], qb[$1], $5/astar[$1], $3}' \
${astar} ${data1} ${data2} ${data4} ${data8} > searchoverhead

gnuplot <<EOF
   set terminal postscript
   set title "Size of Local Buffer and Search Overhead"
   set xlabel "Maxium Local Buffer Size"
   set ylabel "Expansion"
   set logscale x
   set logscale y
   set title font "Arial, 26"
   set xlabel font "Arial, 26"
   set ylabel font "Arial, 26"
   set tics font "Arial, 20"

   set output "analysis/outgo.searchoverhead.ps" 
   plot "searchoverhead" using 4:3 pt 2 title "2 threads"
   plot "searchoverhead" using 6:5 pt 3 title "4 threads"
   plot "searchoverhead" using 8:7 pt 4 title "8 threads"
EOF


# The Maxium Size of Outgo Buffer and Search Overhead: Scatter Plot


#rm spd expd gene
