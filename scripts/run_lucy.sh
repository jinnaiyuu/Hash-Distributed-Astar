#!/bin/bash
# this script translate all strips domain with types to STRIPS without types.
# translating types are not the optimal implementation. Shouldn't do this.

##################################################
## IMPORTANT NOTES
##################################################

# 1. this script is FOR ONE SINGLE EVALUATION.
#    SHOULD NOT use this script for several parameters.
#    SINGLE PARAMETER SETTING for single run.
#    Use one level meta script to make comparison among parameters. 

declare -A domain_file
declare -A instance_files

date=`date +%m%d%H%M`
system=`uname -a | sed -e 's/#//g'`

cd ../src

########################
## Command line arguments
########################

# algorithms
# -a "astar hdastar-N"

# structure hash method
# -s abst-N

# heuristic
# -h h-N

# PDB
# -p pdb-N

# instances
# -t: test run with blocks, gripper, openstacks-strips
# -m: test run with gripper

alg="hdastar-1"
structure="abst-0"
heuristic="h-1"
pdb="pdb-0"
runtime="5m"
i=150
dom=""

while getopts "i:t:d:a:s:h:p:" opt; do
    case $opt in
	a) # alg
	        alg="$OPTARG"
		    ;;
	s) # structured zobrist hash
	        structure="$OPTARG"
		    ;;
	h) # heuristic
	        heuristic="$OPTARG"
		    ;;
	p) # pdb
	        pdb="$OPTARG"
		    ;;
	i) # number of instances to run
	        i=$OPTARG
		    ;;
	t) # time to run each instance
	        runtime="$OPTARG"
		    ;;
	d) # domains to run
	        dom="$OPTARG"
		    ;;
	\?)
	        ;;
    esac
done

n_threads=`echo "$alg" | awk -F '-' '{print $2}'`

if echo $n_threads | egrep -q '^[0-9]+$'; then
    echo
else
    n_threads=1
fi
sim_job=`expr 100 / $n_threads / 2`


if [ $i -ne 150 ]
then
    echo "run $i instances each domain"
fi

if [ "$dom" ]
then
    echo "run only $d domain"
fi

echo "alg=$alg"
echo "simjob=$sim_job"
echo "structure=$structure"
echo "heuristic=$heuristic"
echo "pdb=$pdb"
echo "i=$i"
echo "runtime=$runtime"
echo "dom=$dom"
#exit 0


#########################
## File name conventions
#########################


# TODO: these file name conventions should be easier with regular expression.
#       refactor this after this prototype worked.

###############################
### 1 arg instance file name
###############################

# 1. depot type
# d: domain.pddl
# i: pfileX
d1="depot driverlog freecell zenotravel"
for d in $d1
do
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name pfile[0-9]* | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done

# 2. grid
# d: domain.pddl
# i: probXX.pddl
d2="grid gripper logistics98 movie no-mprime no-mystery nomystery-opt11-strips"
for d in $d2
do
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name prob[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done


# 3. pegsol-opt11-strips
# d: domain.pddl
# i: pXX.pddl
d3="pegsol-opt11-strips rovers scanalyzer-opt11-strips sokoban-opt11-strips storage tpp transport-opt11-strips woodworking-opt11-strips"
for d in $d3
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name p[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done


###############################
### > 1 arg instance file name
###############################

# 4. barman type
# d: domain.pddl
# i: pfileXX-YYY.pddl
d4="barman-opt11-strips parking-opt11-strips"
for d in $d4
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name pfile[0-9]*-[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done


# 5. floortile
# d: domain.pddl
# i: opt-pXX-YYY.pddl
d5="floortile-opt11-strips"
for d in $d5
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name opt-p[0-9]*-[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done


# 6. miconic
# d: domain.pddl
# i: sX-Y.pddl
d6="miconic"
for d in $d6
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name s[0-9]*-[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done


# 7. visitall-opt11-strips
# d: domain.pddl
# i: problemXX-half.pddl problemXX-full.pddl 
d7="visitall-opt11-strips"
for d in $d7
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name problem[0-9]*-*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done


# 8. blocks type
# d: domain.pddl
# i: prob<DOMAINNAME>-X-Y.pddl
# blocks logistics00(logistics)
d8="blocks logistics00"
for d in "blocks"
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name probBLOCKS-[0-9]*-[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done
for d in "logistics00"
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name probLOGISTICS-[0-9]*-[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done


# 9. satellite
# d: domain.pddl
# i: pXX-pfileY.pddl, pXX-HC-pfileY.pddl
d9="satellite"
for d in $d9
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name p[0-9]*-*pfile[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done

# 10. pipesworld-notankage
# d: domain.pddl
# i: pXX-netY-bZ-gW.pddl
d10="pipesworld-notankage"
for d in $d10
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name p[0-9]*-net[0-9]*-b[0-9]*-g[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done

# 11. pipesworld-tankage
# d: domain.pddl
# i: pXX-netY-bZ-gW-tV.pddl
d11="pipesworld-tankage"
for d in $d11
do
#    echo $d
    domain_file+=( 
	[${d}]="domain.pddl"
    )
    is=`find ../pddl/${d} -name p[0-9]*-net[0-9]*-b[0-9]*-g[0-9]*-t[0-9]*.pddl | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
done

###############################
### domains with multiple domain files
###############################

# 12. elevator types
# d: pXX-domain.pddl
# i: pXX.pddl
d12="elevators-opt08-strips openstacks-opt08-strips openstacks-opt11-strips scanalyzer-08-strips parcprinter-08-strips parcprinter-opt11-strips pegsol-08-strips sokoban-opt08-strips transport-opt08-strips woodworking-opt08-strips"
for d in $d12
do
    ds=`find ../pddl/${d} -regex "../pddl/${d}/p[0-9]*-domain.pddl" | sort -V`
    domain_file+=( 
	[${d}]=${ds}
    )
    is=`find ../pddl/${d} -regex "../pddl/${d}/p[0-9]*.pddl" | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
#    echo "ds = $ds"
done

# 13. openstacks-strips
# d: domain_pXX.pddl
# i: pXX.pddl
d13="openstacks-strips trucks-strips"
for d in $d13
do
    ds=`find ../pddl/${d} -regex "../pddl/${d}/domain_p[0-9]*.pddl" | sort -V`
    domain_file+=( 
	[${d}]=${ds}
    )
    is=`find ../pddl/${d} -regex "../pddl/${d}/p[0-9]*.pddl" | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
#    echo "ds = $ds"
done


# 14. airport type
# d: pXX-domain.pddl
# i: pXX-airportYY-pZ.pddl pXX-airportYYhalfMUC-pZ.pddl pXX-airportYYMUC-pZ.pddl
d14="airport"
for d in $d14
do
    ds=`find ../pddl/${d} -regex "../pddl/${d}/p[0-9]*-domain.pddl" | sort -V`
    domain_file+=( 
	[${d}]=${ds}
    )
    is=`find ../pddl/${d} -regextype posix-egrep -regex "../pddl/${d}/p[0-9]*-airport[0-9]*(|halfMUC|MUC)-p[0-9]*.pddl" | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
#    echo "ds = $ds"
done

# 15. psr-small
# d: pXX-domain.pddl
# i: pXX-sY-nZ-lW-fV.pddl
d15="psr-small"
for d in $d15
do
    ds=`find ../pddl/${d} -regex "../pddl/${d}/p[0-9]*-domain.pddl" | sort -V`
    domain_file+=( 
	[${d}]=${ds}
    )
    is=`find ../pddl/${d} -regex "../pddl/${d}/p[0-9]*-s[0-9]*-n[0-9]*-l[0-9]*-f[0-9]*.pddl" | sort -V`
    instance_files+=(
	[${d}]=${is}
    )
#    echo "ds = $ds"
done



# all domains
# ""

### domains to run

domains="$d1 $d2 $d3 $d4 $d5 $d6 $d7 $d8 $d9 $d10 $d11"
domains2="$d12 $d13 $d14 $d15"


### domains to run
if [ "$dom" ]
then
    if [[ $domains == *"$dom"* ]]
    then
	domains=$dom
	domains2=""
    else
	domains=""
	domains2=$dom
    fi
fi


### instances to run

if [ $i -ne 150 ]
then
#    domains="storage blocks rovers gripper"
#    domains2="openstacks-strips psr-small"
    for domain in $domains
    do
	instance_files[${domain}]="`echo ${instance_files[${domain}]} | \
        awk -v i=$i '{t=0; while(t<i) {++t; printf(\"%s\n\", $t);}}'`"
    done
    for domain in $domains2
    do
	domain_file[${domain}]="`echo ${domain_file[${domain}]} | \
        awk -v i=$i '{t=0; while(t<i) {++t; printf(\"%s\n\", $t);}}'`"
	instance_files[${domain}]="`echo ${instance_files[${domain}]} | \
        awk -v i=$i '{t=0; while(t<i) {++t; printf(\"%s\n\", $t);}}'`"
    done
fi


#echo "$domains $domains2"

#if false
#then
for domain in $domains
do
    dfile=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    echo "##################"
    echo "domain: "
    echo "$domain"
    echo "dfile: "
    echo "$dfile"
    echo "ifile: "
    echo "$ifiles"
    echo
done

for domain in $domains2
do
    dfile=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    echo "##################"
    echo "domain: "
    echo "$domain"
    echo "dfile: "
    echo "$dfile"
    echo "ifile: "
    echo "$ifiles"
    echo
done
#fi

#exit 0


###############################
### domains with single domain files
###############################

output_dir="../results/raws/${date}"

mkdir ../results
mkdir ../results/raws
mkdir ../results/raws/${date}

#if false
#then

#alg="hdastar-1"
#structure=abst-1
#heuristic=h-1
#pdb=pdb-0


#############################
### Run Instances
#############################

for domain in $domains
do 
    echo "##################"
    echo "domain: $domain"
    dfile=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    echo "instance: $ifiles"

    for i in $ifiles
    do
	qsub -v arg1="$alg",arg2="../pddl/${domain}/${dfile}",arg3="$i",arg4="$heuristic",arg5="$structure",arg6="$pdb",arg7="$output_dir" ./run.sh
    done
    echo
done

# -l nodes=1:ppn=1,walltime=00:05:00,mem=14gb 
#-M ddyuudd@gmail.com -m ae -N $NAME -j oe 


for domain in $domains2
do
    echo "##################"
    echo "domain: $domain"
    dfiles=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    echo "instance: $ifiles"
    
    size=`echo $dfiles | wc -w`
    i=0
    while [ $i -st $size ]
    do
	qsub -v arg1="$alg",arg2="../pddl/${domain}/${dfile}",arg3="$i",arg4="$heuristic",arg5="$structure",arg6="$pdb",arg7="$output_dir" ./run.sh
    done
    echo
done

exit 0


#############################
### Summarize results to the summary
#############################

for domain in $domains
do 
    echo "##################"
    echo "domain: $domain"
    dfile=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    parallel --noswap --results $output_dir timeout "$runtime" \
	./summarize.sh ::: \
	"$alg" ::: ../pddl/${domain}/${dfile} ::: $ifiles ::: "$heuristic" ::: "$structure" ::: "$pdb" ::: $output_dir
    echo
done
for domain in $domains2
do
    echo "##################"
    echo "domain: $domain"
    dfiles=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    for a in $alg
    do
	for h in $heuristic
	do
	    for s in $structure
	    do
		parallel --noswap --results $output_dir --xapply \
		    timeout "$runtime" ./summarize.sh \
		    ::: "$a" ::: "$dfiles" ::: $ifiles \
		    ::: "$h" ::: "$s" ::: "$pdb" ::: $output_dir
	    done
	done
    done
    echo
done



echo "date=$date, alg=$alg, simjob=$sim_job, structure=$structure, heuristic=$heuristic, pdb=$pdb, runtime=$runtime, system=$system"

parameter_text="
date=$date\n
alg=$alg\n
simjob=$sim_job\n
structure=$structure\n
heuristic=$heuristic\n
pdb=$pdb\n
runtime=$runtime\n
system=$system\n"

echo -e $parameter_text > $output_dir/parameters

echo "summary: "
cat $output_dir/summary  | sort -V > $output_dir/summary_buf
rm $output_dir/summary
mv $output_dir/summary_buf $output_dir/summary

cat $output_dir/summary | ../scripts/print.sh $output_dir # $output_dir/parameters

cat $output_dir/parameters $output_dir/summary | mailx -s "run_all.sh done $date" -A $output_dir/summary.pdf ddyuudd@gmail.com


