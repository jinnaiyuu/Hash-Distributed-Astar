#!/bin/bash

instances=$1
data=$2
instlength=`wc -l ${instances} | awk '{print $1}'`
datalength=`wc -l ${data} | awk '{print $1}'`

echo $instlength $datalength

awk 'BEGIN{n=0}FNR==NR{instance[NR] = $0; next} {order[FNR] = $1; n++} \
END { \
for (i = 1; i <= n; i++) {\
printf("%s\n", instance[order[i]]); }\
}' ${instances} ${data} > ${instances}_big_first

