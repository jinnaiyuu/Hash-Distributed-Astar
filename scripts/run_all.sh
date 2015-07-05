#!/bin/bash
# this script translate all strips domain with types to STRIPS without types.
# translating types are not the optimal implementation. Shouldn't do this.

declare -A domain_file
declare -A instance_files

date=`date +%m%d%H%M`

cd ../src

########################
## Command line arguments
########################

# if -t then 
#    test run with blocks, gripper, openstacks-strips
# if -m then
#    test run with gripper

t=0
m=0

while getopts ":tm" opt; do
    case $opt in
	t)
	        t=1
		    ;;
	m)
	        m=1
		    ;;
	\?)
	        ;;
    esac
done

if [ $t -eq 1 ]
then
    echo "test run"
fi

if [ $m -eq 1 ]
then
    echo "minimal test run"
fi
#echo $t $m 
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

if [ $t -eq 1 ]
then
    domains="blocks gripper"
    domains2="openstacks-strips"
fi

if [ $m -eq 1 ]
then
    domains="gripper"
    domains2=""
fi

#echo "$domains $domains2"

if false
then
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
fi

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

#############################
### Run Instances
#############################

for domain in $domains
do 
    echo "##################"
    echo "domain: $domain"
    dfile=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    parallel --noswap --results $output_dir timeout 5m ./run.sh ::: \
	astar ::: ../pddl/${domain}/${dfile} ::: "$ifiles" ::: "h-1" ::: "abst-1" ::: "pdb-1" ::: $output_dir
    echo
done

for domain in $domains2
do
    echo "##################"
    echo "domain: $domain"
    dfiles=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    parallel --noswap --results $output_dir --xapply timeout 5m ./run.sh \
	::: astar ::: "$dfiles" ::: "$ifiles" ::: "h-1" ::: "abst-1" ::: "pdb-1" ::: $output_dir
    echo
done


#############################
### Summarize results to the summary
#############################

for domain in $domains
do 
    echo "##################"
    echo "domain: $domain"
    dfile=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    parallel --noswap --results $output_dir timeout 5m ./summarize.sh ::: \
	astar ::: ../pddl/${domain}/${dfile} ::: "$ifiles" ::: "h-1" ::: "abst-1" ::: "pdb-1" ::: $output_dir
    echo
done
for domain in $domains2
do
    echo "##################"
    echo "domain: $domain"
    dfiles=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    parallel --noswap --results $output_dir --xapply timeout 5m ./summarize.sh \
	::: astar ::: "$dfiles" ::: "$ifiles" ::: "h-1" ::: "abst-1" ::: "pdb-1" ::: $output_dir
    echo
done




echo "summary: "
cat $output_dir/summary

cat $output_dir/summary | mailx -s "run_all.sh done $date" ddyuudd@gmail.com


#############################
### Print into PDF
#############################

# TODO: output as coverage and node evaluations in table.

# syntax of the input
# <domain> <instance> <walltime> <expanded nodes> <solved> <stage>
# <domain> <instance> <walltime> <expanded nodes> <solved> <stage>
# <domain> <instance> <walltime> <expanded nodes> <solved> <stage>
# ....

# put this into a latex style and print into a pdf.

# syntax of the output
# Domain(#problems)   | solved   | expd(total)
# ----------------------------------------------
# <domain><#problems> | <solved> | <expd(total)>
# <domain><#problems> | <solved> | <expd(total)>
# .... 


cat $output_dir/summary | ../scripts/print.sh > $output_dir/summary_print

