#!/bin/bash


awk '$2=="problem" {printf("%s ",$4)} $3=="wall" {printf("%s " ,$5)} $4=="expanded" {printf("%s " ,$5)} $4=="generated" {printf("%s\n",$5)} $2=="PBS:"{printf("\n")}'

