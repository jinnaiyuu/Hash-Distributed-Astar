#!/bin/bash

# USAGE
# $1 == ALGORITHM

if [ $# -ne 1 ]
then
    echo "Usage: see.sh <algorithm>"
    echo "Pull and visualize the data from Torque server."
fi



# Pull data from server
# Also put the data to archive in the server.
./pull.sh $1 $2


# Parse data into astar.dat
for FILE in $1*.o* 
do

    ./parse.sh < ${FILE} >> $1.dat

done

# visualize with R
# Duplicated valuable. 
./visualize.sh $1 < $1.dat

# Store data in each directory
mv $1*.o* $1

