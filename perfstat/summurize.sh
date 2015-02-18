#!/bin/bash

rm hdastar_perf_sum

echo "hdastar"
for t in 1 2 4 8 12 16
do
    cat hdastar_perf_$t | awk -v t=$t '/cache-misses/{cache=$1} /minor-faults/{page=$1} /elapsed/{printf("%u %s %s %s\n", t, cache, page, $1)}' | tr -d ',' >> hdastar_perf_sum
done

echo ""

rm hdastarsz_perf_sum

echo "hdastar with SZ"
for t in 1 2 4 8 12 16
do
    cat hdastarsz_perf_$t | awk -v t=$t '/cache-misses/{cache=$1} /minor-faults/{page=$1} /elapsed/{printf("%u %s %s %s\n", t, cache, page, $1)}' | tr -d ',' >> hdastarsz_perf_sum
done

gnuplot<<EOF
  set terminal pdf
  set xlabel "threads"
  set ylabel "cache misses"
  plot "hdastar_perf_sum" with line title "HDA*"
  set output "cache-misses_comparison.pdf"
  replot "hdastarsz_perf_sum" with line title "HDA* with SZ"


  plot "hdastar_perf_sum" u 1:3 with line title "HDA*"
  set output "page-faults_comparison.pdf"
  replot "hdastarsz_perf_sum" u 1:3  with line title "HDA* with SZ"

EOF
