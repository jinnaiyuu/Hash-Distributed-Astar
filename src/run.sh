#!/bin/bash

# 1: algorithm
# 2: domain file
# 3: instance file
# 4: heuristic (h-X)
# 5: abst      (abst-X)
# 6: pdb       (pdb-X)
# 7: output file directory



# If it is not in torque env, then assign values
if [ $# -ne 0 ]
then
    arg1=$1
    arg2=$2
    arg3=$3
    arg4=$4
    arg5=$5
    arg6=$6
    arg7=$7
    arg8=$8
else 
    cd /home/jinnai/workspace/strips/src    
fi


d=`echo $arg2 | awk -F / '{print $3}'`
i=`echo $arg3 | awk -F / '{print $4}'`

echo "1=$arg1"
echo "2=$arg2"
echo "3=$arg3"
echo "4=$arg4"
echo "5=$arg5"
echo "6=$arg6"
echo "7=$arg7"
echo "d=$argd"
echo "i=$argi"

filename=`echo "$arg1$d$i$arg4$arg5$arg6"`

./strips.out $arg1 $arg2 $arg3 $arg4 $arg5 $arg6 > "$arg7/$filename"

cat "$arg7/$filename"

