#!/bin/bash

#
# This script is to compare the order of search by plotting in what order each state is opened.
#

#cd /home/yuu/workspace/ethan

if [ $# -ne 0 ]
then 
    dat1=$1
    dat2=$2
    dat3=$3
    dat4=$4
else
    dat1=$arg1
    dat2=$arg2
    dat3=$arg3
    dat4=$arg4
fi

# 1. Parse the input file to get the order data.
echo "start ${dat1}"
head -n 3000000 $dat1 |  awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("1 "); print} } /incumbent/{isData = 1}' | sort -k 4 > ${dat1}.buf
echo "dat1 done"
head -n 3000000 $dat2 |  awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("2 "); print} } /incumbent/{isData = 1}' | sort -k 4 > ${dat2}.buf
echo "dat2 done"
head -n 3000000 $dat3 |  awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("4 "); print} } /incumbent/{isData = 1}' | sort -k 4 > ${dat3}.buf
echo "dat3 done"
head -n 3000000 $dat4 |  awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("8 "); print} } /incumbent/{isData = 1}' | sort -k 4 > ${dat4}.buf
echo "dat4 done"

# 2. Join two input data into a single data. First need to sort to join.

join -j 4 -a1 -a2 ${dat1}.buf ${dat2}.buf | awk 'NF==9{print} NF==5&&$2==1{for(i=1;i<=NF;i++){printf("%s ", $i)}; printf("2 -1 -1 -1 \n")} \
NF==5&&$2==2{printf("%s 1 -1 -1 -1", $1); for(i=2;i<=NF;i++){printf(" %d", $i)}; printf("\n")}' > ${dat1}.2joined

echo "2joined"

join -1 1 -2 4 -a1 -a2 ${dat1}.2joined ${dat3}.buf | awk 'NF==13{print} NF==9{for (i=1;i<=NF;i++){printf("%s ", $i)}; printf("4 -1 -1 -1\n");} \
NF==5{printf("%s 1 -1 -1 -1 2 -1 -1 -1",$1); for(i=2;i<=F;i++){printf(" %d",$i)}; printf("\n")}' > ${dat1}.4joined

echo "4joined"

join -1 1 -2 4 -a1 -a2 ${dat1}.4joined ${dat4}.buf | awk 'NF==17{print} NF==13{for(i=1;i<=NF;i++){printf("%s ", $i)}; printf("8 -1 -1 -1\n")} \
NF==5{printf("%s 1 -1 -1 -1 2 -1 -1 -1 4 -1 -1 -1",$1); for(i=2;i<=F;i++){printf(" %d",$i)}; printf("\n")}' > ${dat1}.joined
echo "joined"

echo "joined done" | mailx -s "done" ddyuudd@gmail.com

exit 0
