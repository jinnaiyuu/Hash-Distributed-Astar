#!/bin/bash

#
# This script is to compare the order of search by plotting in what order each state is opened.
#

dat1=$1
dat2=$2
dat3=$3
dat4=$4

# 1. Parse the input file to get the order data.
echo "start"
awk 'BEGIN{isData = 0;} {if (isData) {print} } $3=="wall"&&$4=="time"{isData = 0} /incumbent/{isData = 1}' $dat1 | sort -k 2 > ${dat1}.buf
echo "dat1 done"
awk 'BEGIN{isData = 0;} {if (isData) {print} } $3=="wall"&&$4=="time"{isData = 0} /incumbent/{isData = 1}' $dat2 | sort -k 2 > ${dat2}.buf
echo "dat2 done"
awk 'BEGIN{isData = 0;} {if (isData) {print} } $3=="wall"&&$4=="time"{isData = 0} /incumbent/{isData = 1}' $dat3 | sort -k 2 > ${dat3}.buf
echo "dat3 done"
awk 'BEGIN{isData = 0;} {if (isData) {print} } $3=="wall"&&$4=="time"{isData = 0} /incumbent/{isData = 1}' $dat4 | sort -k 2 > ${dat4}.buf
echo "dat4 done"

# 2. Join two input data into a single data. First need to sort to join.

join -j 2 ${dat1}.buf ${dat2}.buf > order.dat | join -1 3 -2 2 - dat3.buf | join -1 4 -2 2 - dat4.buf > ${dat1}order.dat
echo "joined"

# 3. Plot

gnuplot<<EOF
  set terminal png
  set output "${dat1}.order.png"
  plot "${dat1}order.dat" using 2:3 with lines title "$1-$2"
  plot "${dat1}order.dat" using 2:4 with lines title "$1-$3"
  plot "${dat1}order.dat" using 2:5 with lines title "$1-$4"
EOF

echo "done!"
