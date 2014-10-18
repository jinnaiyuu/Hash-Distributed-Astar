#!/bin/sh

cd $PBS_O_WORKDIR
./tiles$arg1 $arg2 $PBS_ARRAYID $arg3 $arg4 < big_first_instances

