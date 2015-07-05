#!/bin/bash

awk '{domains[$1]=$1; \
         n_problem[$1]+=1; \
         n_expd[$1]+=$4;   \
         n_solved[$1]+=$5;  \
         }
         END{\
         for (d in domains) \
            printf("%s %d %d %d\n", d, n_problem[d], n_expd[d], n_solved[d]) \
         }'

