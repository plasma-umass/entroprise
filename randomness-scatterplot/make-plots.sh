#!/bin/bash

# declare -a ALLOCATORS=('dieharder' 'freeguard' 'guarder' 'hoard' 'jemalloc' 'malloc' 'mesh' 'mimalloc' 'scudo' 'smimalloc' 'tcmalloc')
declare -a ALLOCATORS=('freeguard' 'hoard' 'jemalloc' 'malloc' 'mesh' 'mimalloc' 'smimalloc' 'tcmalloc')
# declare -a BENCHMARKS=('bodytrack' 'streamcluster' 'swaptions' 'vips')
declare -a BENCHMARKS=('vips')

for a in "${ALLOCATORS[@]}"
do
    for b in "${BENCHMARKS[@]}"
    do
        eval "python3 plot.py $a $b &"
    done
done
