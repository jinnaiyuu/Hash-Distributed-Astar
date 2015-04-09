#!/bin/bash

fs=`wc -l $1 | awk '{print $1}'`
echo $fs
awk -v fs=$fs '\
NR==FNR{wall1+=$2; expd1+=$3; lb1+=$4; push1+=$5} \
NR==(FNR+fs){wall2+=$2; expd2+=$3; lb2+=$4; push2+=$5} \
NR==(FNR+fs*2){wall3+=$2; expd3+=$3; lb3+=$4; push3+=$5} \
NR==(FNR+fs*3){wall4+=$2; expd4+=$3; lb4+=$4; push4+=$5} \
END{
print "methods & Each & Pair & Block & TwoBlock & Abstraction \\\\ \\hline"
print "walltime & " wall1/fs, "&", wall2/fs, "&", wall3/fs, "&", wall4/fs,  "\\\\";
print "expansion & " expd1/fs, "&", expd2/fs, "&", expd3/fs, "&", expd4/fs, "\\\\";
print "load balance & " lb1/fs, "&", lb2/fs, "&", lb3/fs, "&", lb4/fs, "\\\\";
print "node send ratio & " push1/fs, "&", push2/fs, "&", push3/fs, "&", push4/fs;
}' $1 $2 $3 $4

# NR==(FNR+fs*4){wall5+=$2; expd5+=$3; lb5+=$4; push5+=$5} \
