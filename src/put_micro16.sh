#!/bin/bash
time=`date +%m%d%H%M%S`

make tiles msa
#make strips.out
#cp ./tiles ./tiles$time

#scp  ./tiles supermicro@supermicro:/home/supermicro/workspace/15puzzle/

rsync  ./tiles aflab@supermicro16:/home/aflab/workspace/15puzzle/
rsync  ./msa.out aflab@supermicro16:/home/aflab/workspace/15puzzle/
#rsync  ../src/strips.out aflab@supermicro16:/home/aflab/workspace/strips/src/
#rsync  ../src/run.sh aflab@supermicro16:/home/aflab/workspace/strips/src/
#rsync  ../src/summarize.sh aflab@supermicro16:/home/aflab/workspace/strips/src/
#rsync  ../scripts/*.sh aflab@supermicro16:/home/aflab/workspace/strips/scripts/
