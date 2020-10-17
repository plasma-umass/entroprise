#!/bin/bash

declare -a GLOBAL_OUTPUT_DIR="/nfs/cm/scratch1/emery/msteranka/entroprise-parsec"
declare -a ALLOCATORS=('malloc' 'dieharder' 'freeguard' 'guarder' 'scudo' 'smimalloc' 'jemalloc' 'tcmalloc' 'hoard' 'mesh')
declare -a BENCHMARKS=('blackscholes' 'bodytrack' 'canneal' 'dedup' 'facesim' 'ferret' 'fluidanimate' 'freqmine' 'raytrace' 'streamcluster' 'swaptions' 'vips' 'x264')
declare -a NUM_BENCHMARKS=${#BENCHMARKS[@]}

echo ""
for a in "${ALLOCATORS[@]}"
do
    echo "$a"
    echo "============================================================"
    for b in "${BENCHMARKS[@]}"
    do
        printf "$b: "
        eval "cat $a/$b-$a.out | grep Calculated | tail -1"
    done
    echo ""
done
