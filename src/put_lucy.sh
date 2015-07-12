#!/bin/bash
time=`date +%m%d%H%M%S`

make strips.out
#cp ./tiles ./tiles$time

#scp  ./tiles supermicro@supermicro:/home/supermicro/workspace/15puzzle/
#scp  ./tiles aflab@supermicro16:/home/aflab/workspace/15puzzle/

scp  ../src/strips.out jinnai@funlucy:/home/jinnai/workspace/strips/src/
scp  ../src/run.sh jinnai@funlucy:/home/jinnai/workspace/strips/src/
scp  ../src/summarize.sh jinnai@funlucy:/home/jinnai/workspace/strips/src/
scp  ../scripts/*.sh jinnai@funlucy:/home/jinnai/workspace/strips/scripts/
