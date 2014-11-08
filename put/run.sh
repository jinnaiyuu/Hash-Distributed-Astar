#!/bin/sh


# arg1 = time
#
#

cd $PBS_O_WORKDIR
./tiles$time $algname $PBS_ARRAYID $thread_number $param1 $param2 $param3 < $problem_type

