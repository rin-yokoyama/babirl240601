#!/bin/sh

run='run'
exp='BR-RIBF40'
scrid='33'

com='/usr/babirl/bin/expdbcom'

$com --List --Channel --expname $exp --scalerid $scrid --csv

IFS=$'\n'
list=(`$com --List --Run --expname $exp --runname $run`)

for x in "${list[@]}"
do
 name=`echo $x | cut -d',' -f2`
 number=`echo $x | cut -d',' -f3`
 $com --List --Data --expname $exp --scalerid $scrid --csv --runname $name --runnumber $number
done
