#!/bin/bash

#
# This script is to compare the order of search by plotting in what order each state is opened.
#

#cd /home/yuu/workspace/ethan

if [ $# -ne 0 ]
then 
    dat1=$1
    dat2=$2
    dat3=$3
    dat4=$4
else
    dat1=$arg1
    dat2=$arg2
    dat3=$arg3
    dat4=$arg4
fi

# 1. Parse the input file to get the order data.
#if false
#then 
echo "start ${dat1}"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("1 "); printf("%d %d %d %s %d 0\n", $1,$2,$3,$4,$5)} } /incumbent/{isData = 1}' $dat1 | sort -k 4 > ${dat1}.buf
echo "dat1 done"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("2 "); printf("%d %d %d %s %d 0\n", $1,$2,$3,$4,$5)} } /incumbent/{isData = 1}' $dat2 | sort -k 4 > ${dat2}.buf
echo "dat2 done"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("4 "); printf("%d %d %d %s %d 0\n", $1,$2,$3,$4,$5)} } /incumbent/{isData = 1}' $dat3 | sort -k 4 > ${dat3}.buf
echo "dat3 done"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("8 "); printf("%d %d %d %s %d 0\n", $1,$2,$3,$4,$5)} } /incumbent/{isData = 1}' $dat4 | sort -k 4 > ${dat4}.buf
echo "dat4 done"

# The size of open list for each experiments

#open1=`grep openlist ${dat1}.buf | awk '{for(i=4;i<NF;i++){printf("%d ",$i)}; printf("\n")}'`
grep_get()
{
    grep $1 $2 | awk -v col=$3 '{print $col}'
}

echo "Extracting variables"

expansion20=`grep expansion_distribution ${dat2}  | awk '{print $3}'`
expansion21=$( grep_get expansion_distribution ${dat2} 4 )

expansion30=`grep expansion_distribution ${dat3}  | awk '{print $3}'`
expansion31=`grep expansion_distribution ${dat3}  | awk '{print $4}'`
expansion32=`grep expansion_distribution ${dat3}  | awk '{print $5}'`
expansion33=`grep expansion_distribution ${dat3}  | awk '{print $6}'`

expansion40=`grep expansion_distribution ${dat4}  | awk '{print $3}'`
expansion41=`grep expansion_distribution ${dat4}  | awk '{print $4}'`
expansion42=`grep expansion_distribution ${dat4}  | awk '{print $5}'`
expansion43=`grep expansion_distribution ${dat4}  | awk '{print $6}'`
expansion44=`grep expansion_distribution ${dat4}  | awk '{print $7}'`
expansion45=`grep expansion_distribution ${dat4}  | awk '{print $8}'`
expansion46=`grep expansion_distribution ${dat4}  | awk '{print $9}'`
expansion47=`grep expansion_distribution ${dat4}  | awk '{print $10}'`

duplicate20=$( grep_get duplicate_distribution ${dat2} 3 )
duplicate21=$( grep_get duplicate_distribution ${dat2} 4 )

duplicate30=$( grep_get duplicate_distribution ${dat3} 3 )
duplicate31=$( grep_get duplicate_distribution ${dat3} 4 )
duplicate32=$( grep_get duplicate_distribution ${dat3} 5 )
duplicate33=$( grep_get duplicate_distribution ${dat3} 6 )

duplicate40=$( grep_get duplicate_distribution ${dat4} 3 )
duplicate41=$( grep_get duplicate_distribution ${dat4} 4 )
duplicate42=$( grep_get duplicate_distribution ${dat4} 5 )
duplicate43=$( grep_get duplicate_distribution ${dat4} 6 )
duplicate44=$( grep_get duplicate_distribution ${dat4} 7 )
duplicate45=$( grep_get duplicate_distribution ${dat4} 8 )
duplicate46=$( grep_get duplicate_distribution ${dat4} 9 )
duplicate47=$( grep_get duplicate_distribution ${dat4} 10 )


open20=`grep openlist ${dat2} | awk '{print $4}'`
open21=`grep openlist ${dat2} | awk '{print $5}'`

open30=`grep openlist ${dat3} | awk '{print $4}'`
open31=`grep openlist ${dat3} | awk '{print $5}'`
open32=`grep openlist ${dat3} | awk '{print $6}'`
open33=`grep openlist ${dat3} | awk '{print $7}'`

open40=`grep openlist ${dat4} | awk '{print $4}'`
open41=`grep openlist ${dat4} | awk '{print $5}'`
open42=`grep openlist ${dat4} | awk '{print $6}'`
open43=`grep openlist ${dat4} | awk '{print $7}'`
open44=`grep openlist ${dat4} | awk '{print $8}'`
open45=`grep openlist ${dat4} | awk '{print $9}'`
open46=`grep openlist ${dat4} | awk '{print $10}'`
open47=`grep openlist ${dat4} | awk '{print $11}'`


# Length of the search, which corresponds to the expanded nodes.
# TODO: Duplicated calculation as the expanded nodes are already known.

length1=`wc -l ${dat1}.buf | awk '{print $1}'`
length2=`wc -l ${dat2}.buf | awk '{print $1}'`
length3=`wc -l ${dat3}.buf | awk '{print $1}'`
length4=`wc -l ${dat4}.buf | awk '{print $1}'`

#fi

# 2. Join two input data into a single data. First need to sort to join.
echo "Joining the bufs"

join -j 4 -a1 -a2 ${dat1}.buf ${dat2}.buf | awk 'NF==11{print} NF==6&&$2==1{for(i=1;i<=NF;i++){printf("%s ", $i)}; printf("2 -1 -1 -1 -1 \n")} \
NF==6&&$2==2{printf("%s 1 -1 -1 -1 -1", $1); for(i=2;i<=NF;i++){printf(" %d", $i)}; printf("\n")}' > ${dat1}.2joined
join -1 1 -2 4 -a1 -a2 ${dat1}.2joined ${dat3}.buf | awk 'NF==16{print} NF==11{for (i=1;i<=NF;i++){printf("%s ", $i)}; printf("4 -1 -1 -1 -1\n");} \
NF==6{printf("%s 1 -1 -1 -1 -1 2 -1 -1 -1 -1",$1); for(i=2;i<=F;i++){printf(" %d",$i)}; printf("\n")}' > ${dat1}.4joined
join -1 1 -2 4 -a1 -a2 ${dat1}.4joined ${dat4}.buf | awk 'NF==21{print} NF==16{for(i=1;i<=NF;i++){printf("%s ", $i)}; printf("8 -1 -1 -1 -1\n")} \
NF==6{printf("%s 1 -1 -1 -1 -1 2 -1 -1 -1 -1 4 -1 -1 -1 -1",$1); for(i=2;i<=F;i++){printf(" %d",$i)}; printf("\n")}' > ${dat1}.joined
echo "joined"

if [ $# -eq 0 ]
then
    rm ${dat1}.buf ${dat2}.buf ${dat3}.buf ${dat4}.buf
fi

joinedlength=`wc -l ${dat1}.joined | awk '{print $1}'`
if [ $joinedlength -eq 0 ]
then 
    echo "Trouble"
    exit 1
fi 


# Get Highlights
sed -n '1p' ${dat1}.joined > ${dat1}.first
astarnodes=`awk '{print $4}' ${dat1}.first`
#uniq -D -w 32 ${dat1}.2joined > ${dat1}.2dup
#uniq -D -w 32 ${dat1}.4joined > ${dat1}.4dup
#uniq -D -w 32 ${dat1}.joined > ${dat1}.dup

# 3. Plot

echo "Ready to plot"

gnuplot<<EOF
  set terminal pdf
  set xrange[0:]
  set yrange[0:]
  set xtics 2000
  set ytics 2000
  set size ratio -1
  set key out right
  freq = 100

  plot "${dat1}.joined" using 4:(\$8==0?\$9:1/0) every freq w p pt 0 lc rgb"red" notitle
  replot 1/0 w p pt 7 ps 1 lc rgb"red"  title "Thread 1"
  replot "${dat1}.joined" using 4:(\$8==1?\$9:1/0) every freq  w p pt 0 lc rgb"blue" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"blue" title "Thread 2"

#  set xr[0:40000]
  replot x lw 2 lc "black" title "Strict Order"

  set output "2_threads_draft.pdf"
  replot "${dat1}.first"  using 4:9 w p pt 14 lc "black" ps 4 notitle,\
       1/0  w p pt 14 lc "black" ps 1 title "Goal"
#  set terminal png size 1280,960

#  set title "Thread 1-2 Nodes Searched $astarnodes-$length2"
#  replot "${dat1}.joined" using 4:(\$5!=-1?\$9:1/0) w impulse lt 1 notitle

#       "${dat1}.joined" using 4:(\$8==-1&&\$4<$astarnodes?0:1/0) w p pt 2 notitle,\
#       "${dat1}.joined" using 4:(\$5!=-1?1:1/0):5 w labels notitle,\
#       "${dat1}.joined" using 4:(\$8==0?\$9:1/0):(0):(-\$11) every 30 w xerrorbars pt 0 lc rgb"red" notitle,\
#       "${dat1}.joined" using 4:(\$8==1?\$9:1/0):(0):(-\$11) every 30 w xerrorbars pt 0 lc rgb"blue" notitle,\
#       "${dat1}.2dup"   using 4:9 w p pt -1 ps -1 notitle,\                

#EOF

#evince 2_threads_draft.pdf

#exit 0

#  set title "Thread 1-4 Nodes Searched $astarnodes-$length3"
  set output "4_threads_draft.pdf"
  plot "${dat1}.joined" using 4:(\$13==0?\$14:1/0) every freq w p pt 0 lc rgb"red" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"red" title "Thread 1",\
       "${dat1}.joined" using 4:(\$13==1?\$14:1/0) every freq w p pt 0 lc rgb"blue" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"blue" title "Thread 2",\
       "${dat1}.joined" using 4:(\$13==2?\$14:1/0) every freq w p pt 0 lc rgb"green" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"green" title "Thread 3",\
       "${dat1}.joined" using 4:(\$13==3?\$14:1/0) every freq w p pt 0 lc rgb"yellow" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"yellow" title "Thread 4",\
\
       x lw 2 lc "black" title "Strict Order",\
\
       "${dat1}.first"  using 4:14 w p pt 14 lc "black" ps 4 notitle,\
       1/0  w p pt 14 lc "black" ps 1 title "Goal"

  set output "8_threads_draft.pdf"
  set xtics 10000
  set ytics 10000
  plot "${dat1}.joined" using 4:(\$18==0?\$19:1/0) every freq w p pt 0 lc rgb"red" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"red" title "Thread 1",\
       "${dat1}.joined" using 4:(\$18==1?\$19:1/0) every freq w p pt 0 lc rgb"blue" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"blue" title "Thread 2",\
       "${dat1}.joined" using 4:(\$18==2?\$19:1/0) every freq w p pt 0 lc rgb"green" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"green" title "Thread 3",\
       "${dat1}.joined" using 4:(\$18==3?\$19:1/0) every freq w p pt 0 lc rgb"yellow" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"yellow" title "Thread 4",\
       "${dat1}.joined" using 4:(\$18==4?\$19:1/0) every freq w p pt 0 lc rgb"indigo" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"magenta" title "Thread 5",\
       "${dat1}.joined" using 4:(\$18==5?\$19:1/0) every freq w p pt 0 lc rgb"maroon" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"cyan" title "Thread 6",\
       "${dat1}.joined" using 4:(\$18==6?\$19:1/0) every freq w p pt 0 lc rgb"orange" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"orange" title "Thread 7",\
       "${dat1}.joined" using 4:(\$18==7?\$19:1/0) every freq w p pt 0 lc rgb"teal" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"brown" title "Thread 8",\
\
       x lw 2 lc "black" title "Strict Order",\
\
       "${dat1}.first"  using 4:19 w p pt 14 lc "black" ps 4 notitle,\
       1/0  w p pt 14 lc "black" ps 1 title "Goal"

EOF

evince 8_threads_draft.pdf

exit 0

echo "done!"


if [ $# -eq 0 ]
then
    mv ${dat1}.joined /dat
    rm ${dat1}.first
    rm ${dat1}.2dup ${dat1}.4dup
fi

hdastar.easy_instances.1threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1120213236.o552-7 hdastar.easy_instances.2threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1120002114.o348-7 hdastar.easy_instances.4threads.32gbmem.incomebufsize1000000.outgobufsize1000000.abstraction0.1120002124.o349-7
