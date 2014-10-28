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
    mv $1/$1*.o$2-* ./
fi


<<<<<<< HEAD
# TODO: 
# Name the file here. Well, this should be easy as the raw data files have
# enough information in its name. Just parse it down.

# Parse data into astar.dat
=======

>>>>>>> 6626f80399e81a89e7f7541742efb8a140480ce9
for FILE in $1*.o$2-* 
do
    filename=`echo ${FILE} | awk 'BEGIN{FS = "-"} {print $1}'`
    break
done

# Parse data into astar.dat
for FILE in $1*.o$2-* 
do
    
    ./parse.sh < ${FILE} >> ${filename}
done

# visualize with R
# Duplicated valuable. 
#./visualize.sh $1 < $1.dat

# Store data in each directory
mv $1*.o$2-* $1
