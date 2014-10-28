#!/bin/bash



awk \ '
$2=="problem" {printf("%s ",$4)} \
$3=="wall" {printf("%s " ,$5)} \
$4=="expanded" {printf("%s " ,$5)} \
$4=="generated" {printf("%s ",$5)} \
$2=="solution" {printf("%s", $4)} \
$3=="max_income_buffer_size" {printf("%s ",$5)} \
$3=="max_outgo_buffer_size" {printf("%s ",$5)} \
$1=="duplicated" {printf("%s ", $4)} \
$1=="expansion"&&$2=="stddev" {printf("%s ", $4)} \
$1=="generation"&&$2=="stddev" {printf("%s ", $4)} \
$1=="outsource" {printf("%s ", $5)} \
END {printf("\n")}'

exit 0

awk \ '
$2=="problem" {problem=$4} \
$3=="wall" {wall[problem]=$5} \
$4=="expanded" {expanded[problem]=$5} \
$4=="generated" {generated[problem]=$5} \
$2=="solution" {solution[problem]=$4} \
\
$3=="max_income_buffer_size" {income[problem]=$5} \
$3=="max_outgo_buffer_size" {outgo[problem]=$5} \
$1=="duplicated" {duplicate[problem]=$4} \
$1=="expansion"&&$2=="stddev" {expand_stddev[problem]=$4} \
$1=="generation"&&$2=="stddev" {generate_stddev[problem]=$4} \
\
$1=="outsource" {outsourced_nodes[problem]=$5} \
\
$1=="forcepush" {forcepush[problem]=$3} \
END { \
for (i = 1; i <= 100; ++i) { \
print\
\
}'

