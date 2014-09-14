#!/bin/bash

cd $PBS_O_WORKDIR
echo "Chech this out!" | mailx -s "Lucy: $JOB_ARRAY_ID done." ddyuudd@gmail.com
