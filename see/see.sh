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


for FILE in $1*.o$2-* 
do
    filename=`echo ${FILE} | awk 'BEGIN{FS = "-"} {print $1}'`
    break
done

#echo "problem wall expd gend solution max_income_buffer max_outgo_buffer duplicated expd_stddev gend_stddev outsource income_force outgo_force" > ${filename}

# Parse data into astar.dat
for FILE in $1*.o$2-* 
do
    
    ./parse.sh < ${FILE} >> ${filename}
done


sort -n ${filename} > ${filename}buf

rm ${filename}

cp ${filename}buf ${filename}

rm ${filename}buf

# visualize with R
# Duplicated valuable. 
#./visualize.sh $1 < $1.dat

# Store data in each directory
mv $1*.o$2-* $1
