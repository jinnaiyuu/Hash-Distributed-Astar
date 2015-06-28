#!/bin/bash
# this script translate all strips domain with types to STRIPS without types.
# translating types are not the optimal implementation. Shouldn't do this.

declare -A domain_file
declare -A instance_files

cd ../../src



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
d3="pegsol-opt11-strips rovers scanalyzer-08-strips scanalyzer-opt11-strips sokoban-opt11-strips storage tpp transport-opt11-strips woodworking-opt11-strips"
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
d12="elevators-opt08-strips openstacks-opt08-strips openstacks-opt11-strips parcprinter-08-strips parcprinter-opt11-strips pegsol-08-strips sokoban-opt08-strips transport-opt08-strips woodworking-opt08-strips"
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
# openstacks-strips trucks-strips

# 14. airport type
# d: pXX-domain.pddl
# i: pXX-airportYY-pZ.pddl
# airport 

# 15. psr-small
# d: pXX-domain.pddl
# i: pXX-sY-nZ-lW-fV.pddl
# psr-small





# all domains
# ""

### domains to run

#domains="$d1 $d2 $d3 $d4 $d5 $d6 $d7 $d8 $d9 $d10 $d11"
#domains2="$d12 $d13 $d14 $d15"
#echo "$domains $domains2"
#exit 0

domains="depot gripper"
domains2="elevators-opt08-strips"

###############################
### domains with single domain files
###############################

if false
then
for domain in $domains
do
    dfile=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
    parallel timeout 5s ./strips.out ::: astar ::: ../pddl/${domain}/${dfile} ::: "$ifiles" ::: h-1
done
exit 0
fi

###############################
### domains with multiple domain files
###############################


for domain in $domains2
do
    dfiles=${domain_file[${domain}]}
    ifiles=${instance_files[${domain}]}
#    echo "dfiles = $dfiles" 
#    echo "ifiles = $ifiles" 
#    exit 0
    parallel --results ../pddl/aaa_script --xapply timeout 5s ./strips.out \
	::: astar ::: "$dfiles" ::: "$ifiles" ::: "h-1 h-3"
done
