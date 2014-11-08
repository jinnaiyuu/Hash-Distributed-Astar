#!/bin/bash

instances=$1
data=$2
length=`wc -l ${instances}`

awk 'FNR==NR{instance[NR] = $0; next} {order[FNR] = $1} \
END { \
for (i = 1; i <= $length; i++) \
printf("%s\n", instance[order[i]]) \
}' ${instances} ${data} > ${instances}big_first

awk 'FNR==NR{instance[NR] = $0; next} {order[FNR] = $1} \
END { \
for (i = $length; i >= 1; i--) \
printf("%s\n", instance[order[i]]) \
}' ${instances} ${data} > ${instances}_small_first
