#!/bin/bash
time=`date +%m%d%H%M%S`

make
#cp ./tiles ./tiles$time

#scp  ./tiles supermicro@supermicro:/home/supermicro/workspace/15puzzle/

scp  ./tiles aflab@supermicro16:/home/aflab/workspace/15puzzle/
scp  ../src/strips.out aflab@supermicro16:/home/aflab/workspace/strips/src/
scp  ../src/run.sh aflab@supermicro16:/home/aflab/workspace/strips/src/
scp  ../src/summarize.sh aflab@supermicro16:/home/aflab/workspace/strips/src/
scp  ../scripts/*.sh aflab@supermicro16:/home/aflab/workspace/strips/scripts/
