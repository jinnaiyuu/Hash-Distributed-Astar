#!/bin/bash


awk \ '
$2=="problem" {printf("%s ",$4)} \
$3=="wall" {printf("%s " ,$5)} \
$4=="expanded" {printf("%s " ,$5)} \
$4=="generated" {printf("%s ",$5)} \
$3=="max_income_buffer_size" {printf("%s ",$5)} \
$3=="max_outgo_buffer_size" {printf("%s ",$5)} \
$2=="solution" {printf("%s\n", $4)} \
$2=="PBS:"{printf("\n")}'

