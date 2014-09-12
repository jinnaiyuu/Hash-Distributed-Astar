#!/bin/sh

cd $PBS_O_WORKDIR
echo "Chech this out!" | mail -s "Lucy: $JOB_ARRAY_ID done." ddyuudd@gmail.com
