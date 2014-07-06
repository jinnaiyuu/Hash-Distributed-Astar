#!/bin/bash

# USAGE
# $1 == ALGORITHM


rm $1*.o*
rm $1.dat


# Pull data from server
./see/pull.sh $1


# Parse data into astar.dat
for FILE in $1*.o* 
do

    ./see/parse.sh < ${FILE} >> $1.dat

done

# visualize with R
./see/visualize.sh $1 < astar.dat

# Delete 
mv $1*.o* $1

