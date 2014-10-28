#!/bin/bash


# Plots the Speedup of the HDA*

date=`date +"%m%d"`

astar=astar.dat
hda=hdastar_168036.dat
diff2=hdastar_168615.dat
diff4= hdastar_*******.dat

# Two awk scripts can be piped. For debugging purpose, we save the intermediate data structure.
#awk 'FNR==NR{astar[$1]=$2; next} (FNR+100)==NR{print astar[$1], $4}' astar.dat hdastar_1_opt.dat > speedup.dat


awk 'FNR==NR{astar[$1]=$2; next} (FNR+100)==NR{a[$1]=$2; next} (FNR+200)==NR{b[$1]=$2; next} \
(FNR+300)==NR{\
if (length(astar[$1]) != 0) \
if (length(a[$1]) != 0) \
if (length(b[$1]) != 0) \
if (length($2) != 0) \
print astar[$1], a[$1], b[$1], $2}' ${astar} ${hda} ${diff2} ${diff4} > diff2_speedup.dat


gnuplot <<EOF
   set title "Comparison of JOHDA* relative to HDA*"
   set xlabel "JOHDA* (diff = 2)"
   set ylabel "HDA*"   
   set terminal postscript color
   plot "diff2_speedup.dat" using 2:3 with lp
   set output "analysis/speedup_of_diff2_johda_over_astar.ps"
   replot x with line ls 1
EOF

# Speedup over single core HDA*
awk '{ single+= $2; quadra+=$3; oct+=$4; } END{ printf("1 %f\n4 %f\n8 %f", 1, single/quadra, single/oct)}' \
speedup.dat > spd

gnuplot <<EOF
   set title "Speedup of JOHDA* compared to single thread JOHDA*"
   set xlabel "Number of cores"
   set ylabel "Speedup"   
   set terminal postscript color
   plot "spd" using 1:2 with lp
   set output "analysis/speedup_of_johda_over_single_core_johdastar.ps"
   replot x with line ls 1
EOF

exit 0

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
   set ylabel "Average Expansion Ratio (Compared to 1 thread)"   
   set terminal postscript
   set output "analysis/expansion.ps"
   plot "expd" using 1:2 with lp
EOF
exit 0

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
   set ylabel "income buffer size"   
   set terminal postscript
   plot "$data1" using (1):2 with boxplot
   replot "$data2" using (2):2 with boxplot
   replot "$data4" using (4):2 with boxplot 
   set output "analysis/income.ps"
   replot "$data8" using (8):2 with boxplot 
EOF

gnuplot <<EOF
   set ylabel "outgo buffer size"   
   set terminal postscript
   plot "$data1" using (1):3 with boxplot
   replot "$data2" using (2):3 with boxplot
   replot "$data4" using (4):3 with boxplot 
   set output "analysis/outgo.ps"
   replot "$data8" using (8):3 with boxplot 
EOF


#rm spd expd gene
