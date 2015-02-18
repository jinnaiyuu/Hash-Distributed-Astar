#!/bin/bash


dir="/media/yuu/2tb/"

# easy
file1="hdastar.easy_instances.1threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1120213236.o552"
file2="hdastar.easy_instances.2threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1120002114.o348"
file4="hdastar.easy_instances.4threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1120002124.o349"
file8="hdastar.easy_instances.8threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1120002138.o350"

rm ${file1}.2.bind
rm ${file1}.4.bind
rm ${file1}.8.bind

for i in {1..100}
do
    ./order.sh ${dir}$file1-${i} ${dir}$file2-${i} ${dir}$file4-${i} ${dir}$file8-${i}
#    mv ${dir}${file1}-${i}.joined ./easy_instances/

    head -n 5000000 ${dir}${file1}-${i}.joined | awk -v i=$i '$4!=-1&&$8!=-1{sum+= ($4-$8)*($4-$8); num++} END{print i, num, sqrt(sum/num)}' >> $file1.2.bind
    head -n 5000000 ${dir}${file1}-${i}.joined | awk -v i=$i '$4!=-1&&$8!=-1{sum+= ($4-$12)*($4-$12); num++} END{print i, num, sqrt(sum/num)}' >> $file1.4.bind
    head -n 5000000 ${dir}${file1}-${i}.joined | awk -v i=$i '$4!=-1&&$8!=-1{sum+= ($4-$16)*($4-$16); num++} END{print i, num, sqrt(sum/num)}' >> $file1.8.bind
done

sort -n $file1.2.bind | join easy1 - | awk '{print $3, $17}' > ${file1}.2.plot
sort -n $file1.4.bind | join easy1 - | awk '{print $3, $17}' > ${file1}.4.plot
sort -n $file1.8.bind | join easy1 - | awk '{print $3, $17}' > ${file1}.8.plot

./plot.sh ${file1}.2.plot
./plot.sh ${file1}.4.plot
./plot.sh ${file1}.8.plot
