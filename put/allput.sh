#!/bin/sh

SCRIPT="cd /home/jinnai/workspace/ethan ; ./alljob.sh $1"


cd src
make
cd ..
scp  ./src/tiles alljob.sh allrun.sh ./src/instances jinnai@funlucy:/home/jinnai/workspace/ethan/

ssh -l jinnai funlucy "${SCRIPT}"