#!/bin/sh

# See job_xeon.sh for details.

cd $PBS_O_WORKDIR
./tiles$1 $2 $5 $3 $4 < $6

