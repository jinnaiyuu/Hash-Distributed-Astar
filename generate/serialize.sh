#!/bin/bash
input=$1

awk '{printf("%d ",NR); for (i=2; i<=NF; i++) printf $i " "; printf("\n")}' ${input}
