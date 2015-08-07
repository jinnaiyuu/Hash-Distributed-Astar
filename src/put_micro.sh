#!/bin/bash
time=`date +%m%d%H%M%S`

make strips.out
make pstrips.out
make tiles
#cp ./tiles ./tiles$time

scp  ./tiles supermicro@supermicro:/home/supermicro/workspace/15puzzle/
scp  ../src/strips.out supermicro@supermicro:/home/supermicro/workspace/strips/src/
scp  ../src/pstrips.out supermicro@supermicro:/home/supermicro/workspace/strips/src/
scp  ../src/run.sh supermicro@supermicro:/home/supermicro/workspace/strips/src/
scp  ../src/summarize.sh supermicro@supermicro:/home/supermicro/workspace/strips/src/
scp  ../scripts/*.sh supermicro@supermicro:/home/supermicro/workspace/strips/scripts/
