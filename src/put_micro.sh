#!/bin/bash
time=`date +%m%d%H%M%S`

make
#cp ./tiles ./tiles$time

scp  ./tiles supermicro@supermicro:/home/supermicro/workspace/15puzzle
