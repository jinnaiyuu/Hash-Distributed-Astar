#!/bin/sh

cd $PBS_O_WORKDIR
./tiles $arg1 $arg2 $arg3 < instances

