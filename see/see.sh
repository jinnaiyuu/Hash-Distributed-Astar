#!/bin/bash

# USAGE
# $1 == ALGORITHM



if [ $# -ne 2 ]
then
    if [ $# -ne 3 ]
    then
	echo "Usage: see.sh <algorithm> <job id>"
	echo "Pull and visualize the data from Torque server."
	echo "You have to explicitly tell which job to extract by job id."
    fi
fi


if [ $# -eq 2 ]
then
    # Pull data from server
    # Also put the data to archive in the server.
    echo "Pull"
    ./pull.sh $1 $2
fi


# pull from local
if [ $# -eq 3 ]
then 
    echo "From local"
    mv hdastar/$1*.o$2-* ./
fi


# TODO: 
# Name the file here. Well, this should be easy as the raw data files have
# enough information in its name. Just parse it down.

# Parse data into astar.dat
for FILE in $1*.o$2-* 
do

    ./parse.sh < ${FILE} >> $1_$2.dat
done

# visualize with R
# Duplicated valuable. 
#./visualize.sh $1 < $1.dat

# Store data in each directory
mv $1*.o$2-* $1
