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
