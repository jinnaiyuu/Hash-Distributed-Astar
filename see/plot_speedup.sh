#!/bin/bash



data1=hdastar_1_opt.dat
data2=hdastar_2_opt.dat
data4=hdastar_4_opt.dat
data8=hdastar_8_opt.dat

awk 'FNR==NR{single+=$4; next} (FNR+100)==NR{duo+=$4; next} \
(FNR+200)==NR{quadra+=$4; next} (FNR+300)==NR{oct+=$4; next} \
END{printf("1 %f\n2 %f\n4 %f\n 8 %f\n", 1.0, single/duo, single/quadra, single/oct)}' \
${data1} ${data2} ${data4} ${data8} > speedup_amr.dat

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Speedup (Compared to 1 thread hdastar)"   
   set terminal postscript
   set output "analysis/speedup_amr.ps"
   plot "speedup_amr.dat" using 1:2 with lp
EOF



awk 'FNR==NR{a[$1]=$4; next} (FNR+100)==NR{b[$1]=$4; next} (FNR+200)==NR{c[$1]=$4; next} \
{print a[$1]/b[$1], a[$1]/c[$1], a[$1]/$4}' ${data1} ${data2} ${data4} ${data8} > speedup.dat

awk 'FNR==NR{a[$1]=$5; next} (FNR+100)==NR{b[$1]=$5; next} (FNR+200)==NR{c[$1]=$5; next} \
{print a[$1]/b[$1], a[$1]/c[$1], a[$1]/$5}' ${data1} ${data2} ${data4} ${data8} > expd_overhead.dat

awk 'FNR==NR{a[$1]=$6; next} (FNR+100)==NR{b[$1]=$6; next} (FNR+200)==NR{c[$1]=$6; next} \
{print a[$1]/b[$1], a[$1]/c[$1], a[$1]/$6}' ${data1} ${data2} ${data4} ${data8} > gene_overhead.dat

awk 'BEGIN{datanum=100.0} /inf/{datanum--; next} /nan/{datanum--; next} \
{ duo+=$1; quadra+=$2; oct+=$3; } END{ printf("2 %f\n4 %f\n8 %f", duo/datanum, quadra/datanum, oct/datanum)}' speedup.dat > spd

awk 'BEGIN{datanum=100.0} /inf/{datanum--; next} /nan/{datanum--; next} \
{ duo+=$1; quadra+=$2; oct+=$3; } END{ printf("2 %f\n4 %f\n8 %f", duo/datanum, quadra/datanum, oct/datanum)}' expd_overhead.dat > expd

awk 'BEGIN{datanum=100.0} /inf/{datanum--; next} /nan/{datanum--; next} \
{ duo+=$1; quadra+=$2; oct+=$3; } END{ printf("2 %f\n4 %f\n8 %f", duo/datanum, quadra/datanum, oct/datanum)}' gene_overhead.dat > gene


gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Speedup (Compared to 1 thread hdastar)"   
   set terminal postscript
   set output "analysis/speedup.ps"
   plot "spd" using 1:2 with lp
EOF

gnuplot <<EOF
   set xlabel "Number of cores"
   set ylabel "Average Generation Ratio (Compared to 1 thread)"   
   set terminal postscript
   set output "analysis/expansion.ps"
   plot "expd" using 1:2 with lp
EOF

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
