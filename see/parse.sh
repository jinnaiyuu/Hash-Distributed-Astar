#!/bin/bash


awk \ '
$1=="#start" { \
problem = -1; \ 
wall = -1;\
expd = -1;\
gend = -1;\
soltion = -1;\
max_income_buffer = -1;\
max_outgo_buffer = -1;\
duplicated = -1;\
expd_stddev = -1;\
gend_stddev = -1;\
outsource = -1;\
income_force = -1;\
outgo_force = -1;\
}\
$2=="problem" {problem = $4} \
$3=="wall" {wall = $5} \
$4=="expanded" {expd = $5} \
$4=="generated" {gend = $5} \
$2=="solution" {solution = $4} \
$3=="max_income_buffer_size" {max_income_buffer = $5} \
$3=="max_outgo_buffer_size"  {max_outgo_buffer = $5} \
$1=="duplicated" {duplicated = $4} \
$1=="expansion"&&$2=="stddev" {expd_stddev = $4} \
$1=="generation"&&$2=="stddev" {gend_stddev = $4} \
$1=="outsource" {outsource = $5} \
$1=="forcepush"&&$2=="incomebuffer" {income_force = $4} \
$1=="forcepush"&&$2=="outgobuffer" {outgo_force = $4} \
$1=="#end"{printf("\n")}'

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

