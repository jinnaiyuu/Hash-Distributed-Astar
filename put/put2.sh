#!/bin/sh

SCRIPT="cd /home/jinnai/workspace/ethan ; ./job2.sh " 


make
scp  tiles job.sh run.sh instances jinnai@funlucy:/home/jinnai/workspace/ethan/

ssh -l jinnai funlucy "${SCRIPT}"