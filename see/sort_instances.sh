#!/bin/bash

awk 'FNR==NR{instance[NR] = $0; next} {order[FNR] = $1} \
END { \
for (i = 1; i <= 100; i++) \
printf("%s\n", instance[order[i]]) \
}' instances hdastar_sort.dat > big_first_instances

awk 'FNR==NR{instance[NR] = $0; next} {order[FNR] = $1} \
END { \
for (i = 100; i >= 1; i--) \
printf("%s\n", instance[order[i]]) \
}' instances hdastar_sort.dat > small_first_instances
