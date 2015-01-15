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
echo "start ${dat1}"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("1 "); print} } /incumbent/{isData = 1}' $dat1 | sort -k 4 > ${dat1}.buf
echo "dat1 done"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("2 "); print} } /incumbent/{isData = 1}' $dat2 | sort -k 4 > ${dat2}.buf
echo "dat2 done"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("4 "); print} } /incumbent/{isData = 1}' $dat3 | sort -k 4 > ${dat3}.buf
echo "dat3 done"
awk 'BEGIN{isData = 0;} $3=="wall"&&$4=="time"{isData = 0} $1=="openlist"{isData = 0} $1!="incumbent"{if (isData) {printf("8 "); print} } /incumbent/{isData = 1}' $dat4 | sort -k 4 > ${dat4}.buf
echo "dat4 done"

# The size of open list for each experiments

#open1=`grep openlist ${dat1}.buf | awk '{for(i=4;i<NF;i++){printf("%d ",$i)}; printf("\n")}'`
grep_get()
{
    grep $1 $2 | awk -v col=$3 '{print $col}'
}

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

# 2. Join two input data into a single data. First need to sort to join.

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
    exit 1
fi 


# Get Highlights
sed -n '1p' ${dat1}.joined > ${dat1}.first
astarnodes=`awk '{print $4}' ${dat1}.first`
uniq -D -w 32 ${dat1}.2joined > ${dat1}.2dup
uniq -D -w 32 ${dat1}.4joined > ${dat1}.4dup
uniq -D -w 32 ${dat1}.joined > ${dat1}.dup

# 3. Plot

gnuplot<<EOF
  set terminal png size 1280,960
  set yrange[0:]
  set size ratio -1
  set key out vert 
  set key center top
  set title "Thread 1-2 Nodes Searched $astarnodes-$length2"
  set output "${dat1}.order.1.2.png"

  plot "${dat1}.joined" using 4:(\$8==0?\$9:1/0):(0):(-\$11) every 30 w xerrorbars pt 0 lc rgb"red" notitle,\
       "${dat1}.joined" using 4:(\$8==0?\$9:1/0) w p pt 0 lc rgb"red" notitle,\
       1/0  w p pt 7 ps 1 lc rgb"red"  title sprintf("expd %d open %d dup %d sum %d", $expansion20, $open20, $duplicate20, $expansion20 + $open20 + $duplicate20),\
       "${dat1}.joined" using 4:(\$8==1?\$9:1/0):(0):(-\$11) every 30 w xerrorbars pt 0 lc rgb"blue" notitle,\
       "${dat1}.joined" using 4:(\$8==1?\$9:1/0) w p pt 0 lc rgb"blue" notitle,\
       1/0  w p pt 7 ps 1 lc rgb"blue" title sprintf("expd %d open %d dup %d sum %d", $expansion21, $open21, $duplicate21, $expansion21 + $open21 + $duplicate21),\
       "${dat1}.joined" using 4:(\$8==-1&&\$4<$astarnodes?0:1/0) w p pt 2 notitle,\
       "${dat1}.joined" using 4:(\$5!=-1?\$9:1/0) w impulse lt 1 notitle,\
       "${dat1}.joined" using 4:(\$5!=-1?1:1/0):5 w labels notitle,\
       "${dat1}.first"  using 4:9 w p pt 6 ps 5 notitle
#       "${dat1}.2dup"   using 4:9 w p pt -1 ps -1 notitle,\

  set title "Thread 1-4 Nodes Searched $astarnodes-$length3"
  set output "${dat1}.order.1.4.png"
  plot "${dat1}.joined" using 4:(\$13==0?\$14:1/0):(0):(-\$16) every 50 w xerrorbars pt 0 lc rgb"red" notitle, \
       "${dat1}.joined" using 4:(\$13==0?\$14:1/0) w p pt 0 lc rgb"red" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"red"  title sprintf("expd %d open %d dup %d sum %d", $expansion30, $open30, $duplicate30, $expansion30 + $open30 + $duplicate30),\
       "${dat1}.joined" using 4:(\$13==1?\$14:1/0):(0):(-\$16) every 50 w xerrorbars pt 0 lc rgb"blue" notitle, \
       "${dat1}.joined" using 4:(\$13==1?\$14:1/0) w p pt 0 lc rgb"blue" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"blue" title sprintf("expd %d open %d dup %d sum %d", $expansion31, $open31, $duplicate31, $expansion31 + $open31 + $duplicate31),\
       "${dat1}.joined" using 4:(\$13==2?\$14:1/0):(0):(-\$16) every 50 w xerrorbars pt 0 lc rgb"green" notitle, \
       "${dat1}.joined" using 4:(\$13==2?\$14:1/0) w p pt 0 lc rgb"green" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"green" title sprintf("expd %d open %d dup %d sum %d", $expansion32, $open32, $duplicate32, $expansion32 + $open32 + $duplicate32),\
       "${dat1}.joined" using 4:(\$13==3?\$14:1/0):(0):(-\$16) every 50 w xerrorbars pt 0 lc rgb"yellow" notitle, \
       "${dat1}.joined" using 4:(\$13==3?\$14:1/0) w p pt 0 lc rgb"yellow" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"yellow" title sprintf("expd %d open %d dup %d sum %d", $expansion33, $open33, $duplicate33, $expansion33 + $open33 + $duplicate33),\
       "${dat1}.joined" using 4:(\$13==-1&&\$4<$astarnodes?0:1/0) w p pt 2 notitle,\
       "${dat1}.joined" using 4:(\$5!=-1?\$14:1/0) w impulse lt 1 notitle,\
       "${dat1}.joined" using 4:(\$5!=-1?1:1/0):5 w labels notitle,\
       "${dat1}.first"  using 4:14 w p pt 6 ps 5 notitle
#       "${dat1}.4dup"   using 4:14 w p pt -1 ps 2 notitle,\

  set title "Thread 1-8 Nodes Searched $astarnodes-$length4"
  set output "${dat1}.order.1.8.png"
  plot "${dat1}.joined" using 4:(\$18==0?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"red" notitle, \
       "${dat1}.joined" using 4:(\$18==0?\$19:1/0) w p pt 0 lc rgb"red" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"red" title sprintf("expd %d open %d dup %d sum %d", $expansion40, $open40, $duplicate40, $expansion40 + $open40 + $duplicate40),\
       "${dat1}.joined" using 4:(\$18==1?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"blue" notitle, \
       "${dat1}.joined" using 4:(\$18==1?\$19:1/0) w p pt 0 lc rgb"blue" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"blue" title sprintf("expd %d open %d dup %d sum %d", $expansion41, $open41, $duplicate41, $expansion41 + $open41 + $duplicate41),\
       "${dat1}.joined" using 4:(\$18==2?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"green" notitle, \
       "${dat1}.joined" using 4:(\$18==2?\$19:1/0) w p pt 0 lc rgb"green" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"green" title sprintf("expd %d open %d dup %d sum %d", $expansion42, $open42, $duplicate42, $expansion42 + $open42 + $duplicate42),\
       "${dat1}.joined" using 4:(\$18==3?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"yellow" notitle, \
       "${dat1}.joined" using 4:(\$18==3?\$19:1/0) w p pt 0 lc rgb"yellow" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"yellow" title sprintf("expd %d open %d dup %d sum %d", $expansion43, $open43, $duplicate43, $expansion43 + $open43 + $duplicate43),\
       "${dat1}.joined" using 4:(\$18==4?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"indigo" notitle, \
       "${dat1}.joined" using 4:(\$18==4?\$19:1/0) w p pt 0 lc rgb"indigo" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"magenta" title sprintf("expd %d open %d dup %d sum %d", $expansion44, $open44, $duplicate44, $expansion44 + $open44 + $duplicate44),\
       "${dat1}.joined" using 4:(\$18==5?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"maroon" notitle, \
       "${dat1}.joined" using 4:(\$18==5?\$19:1/0) w p pt 0 lc rgb"maroon" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"cyan" title sprintf("expd %d open %d dup %d sum %d", $expansion45, $open45, $duplicate45, $expansion45 + $open45 + $duplicate45),\
       "${dat1}.joined" using 4:(\$18==6?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"orange" notitle, \
       "${dat1}.joined" using 4:(\$18==6?\$19:1/0) w p pt 0 lc rgb"orange" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"orange" title sprintf("expd %d open %d dup %d sum %d", $expansion46, $open46, $duplicate46, $expansion46 + $open46 + $duplicate46),\
       "${dat1}.joined" using 4:(\$18==7?\$19:1/0):(0):(-\$21) every 20 w xerrorbars pt 0 lc rgb"teal"   notitle, \
       "${dat1}.joined" using 4:(\$18==7?\$19:1/0) w p pt 0 lc rgb"teal" notitle,\
       1/0 w p pt 7 ps 1 lc rgb"brown" title sprintf("expd %d open %d dup %d sum %d", $expansion47, $open47, $duplicate47, $expansion47 + $open47 + $duplicate47),\
       "${dat1}.joined" using 4:(\$18==-1&&\$4<$astarnodes?0:1/0) w p pt 2 notitle,\
       "${dat1}.joined" using 4:(\$5!=-1?\$19:1/0) w impulse lt 1 notitle,\
       "${dat1}.joined" using 4:(\$5!=-1?1:1/0):5 w labels notitle,\
       "${dat1}.first"  using 4:19 w p pt 6 ps 5 notitle
#       "${dat1}.dup"    using 4:19 w p pt -1 ps 2 notitle,\

EOF

echo "done!"


if [ $# -eq 0 ]
then
    mv ${dat1}.joined /dat
    rm ${dat1}.first
    rm ${dat1}.2dup ${dat1}.4dup
fi

