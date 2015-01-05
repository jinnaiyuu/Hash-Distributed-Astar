#!/bin/bash


# Input file name as the first argument.
# <num> <walltime> <expansion>

file=$1


# Wall time for each instance

cat ${file} | sort -n -k 2 > ${file}.timesort

cat ${file}.timesort | awk 'BEGIN{printf("0 0\n"); rate=0} NF>=2&&$2<9990{printf("%d %f\n", rate, $2); rate++;}' > ${file}.correctrate


#gnuplot<<EOF
#   set terminal postscript
#   set output "${file}.ps"
#   plot "${file}.correctrate" u 2:1 w l
#EOF

# Cumilative wall time

cat ${file} | awk 'BEGIN{printf("0 0\n"); cumwall=0.0} $2<9990{cumwall=cumwall + $2; printf("%d %f\n", NR, cumwall)}' > ${file}.correctrate.cum

#gnuplot<<EOF
#   set terminal postscript
#   set output "${file}.ps"
#   plot "${file}.correctrate" u 2:1 w l
#EOF


cat ${file} | sort -n -k 3 > ${file}.expdsort

cat ${file}.expdsort | awk 'BEGIN{printf("0 0\n"); pnum=1;} $2<9990{printf("%d %f\n", pnum, $3); pnum++;}' > ${file}.correctrate.expd

#gnuplot<<EOF
#   set terminal postscript
#   set output "${file}.expd.ps"
#   plot "${file}.correctrate.expd" u 2:1 w l
#EOF

#rm ${file}.timesort ${file}.correctrate.cum ${file}.expdsort ${file}.correctrate.expd
