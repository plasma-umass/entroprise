#!/bin/bash

if [ $# != 3 ]
then
    echo "usage: ./perf.sh <approx=y/n> <perfpath> <datapath>"
    exit -1
fi

nobjs=(1000 10000 100000 1000000)
target="./entroprise-standalone"
# allocator="/nfs/cm/scratch1/emery/msteranka/shared/libguarder.so"
# allocator="/nfs/cm/scratch1/emery/msteranka/shared/libscudo.so"
# allocator="/nfs/cm/scratch1/emery/msteranka/shared/libdieharder.so"
# allocator="/nfs/cm/scratch1/emery/msteranka/shared/libsmimalloc.so"
allocator="/nfs/cm/scratch1/emery/msteranka/shared/libfreeguard.so"
objsize=16
nthreads=8
approx=$1
bitwidth=10
perfpath=$2
datapath=$3
timepath="/usr/bin/time"

writemetadata() {
    echo "Allocator: $allocator" > $1
    echo "Object Size: $objsize" >> $1
    echo "Number of Threads: $nthreads" >> $1
    if [ $approx = 'y' ]
    then
        echo "Approximated: yes" >> $1
        echo "Bit Width: $bitwidth" >> $1
    else
        echo "Approximated: no" >> $1
    fi
    echo "" >> $1
}

writemetadata $perfpath
writemetadata $datapath

for i in "${nobjs[@]}"
do
    echo "LD_PRELOAD=$allocator $timepath -f \"E\" $target $objsize $i $nthreads $approx $bitwidth"
    echo -n "$i ALLOCATIONS: " >> $perfpath
    echo -n "$i ALLOCATIONS: " >> $datapath
    LD_PRELOAD=$allocator $timepath -f "%E" $target $objsize $i $nthreads $approx $bitwidth 1>> $datapath 2>> $perfpath
done
