#!/bin/sh

cd $PBS_O_WORKDIR
./tiles $arg1 $PBS_ARRAYID $arg2 < instances
