#!/bin/bash

# USAGE
# $1 == ALGORITHM

if [ $# -ne 1]
then
    echo "Usage: see.sh <algorithm>"
    echo "Pull and visualize the data from Torque server."
fi

rm $1*.o*
rm $1.dat


# Pull data from server
# Also put the data to archive in the server.
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

