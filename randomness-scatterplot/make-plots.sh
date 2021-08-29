#!/bin/bash

declare -a ALLOCATORS=('dieharder' 'freeguard' 'guarder' 'hoard' 'jemalloc' 'malloc' 'mesh' 'mimalloc' 'scudo' 'smimalloc' 'tcmalloc')
declare -a BENCHMARKS=('bodytrack' 'swaptions' 'vips')
c_pids=()
DATA_PATH="/home/msteranka/entroprise-parsec-local"
OUT_PATH="./test-local"

for a in "${ALLOCATORS[@]}"
do
    for b in "${BENCHMARKS[@]}"
    do
        # eval "python3 plot.py $a $b &"
        if [ $b = 'swaptions' ] || [ $b = 'vips' ]; then
            i=1
        else
            i=0
        fi
        while :
        do
            f_name=$DATA_PATH/$a/$b-$a-$i.threads.bin
            if ! [ -f $f_name ]; then
                break
            fi
            python3 plot.py $a $b $i $f_name &
            c_pids+=($!)
            i=$(( i+1 ))
        done
    done
done

for c in "${c_pids[@]}"
do
    wait $c
done

for b in "${BENCHMARKS[@]}"
do
    mkdir -p $OUT_PATH/$b
done

for a in "${ALLOCATORS[@]}"
do
    for b in "${BENCHMARKS[@]}"
    do
        if [ $b = 'swaptions' ] || [ $b = 'vips' ]; then
            i=1
        else
            i=0
        fi
        while :
        do
            f_name=$i-$a-$b.png
            if ! [ -f $f_name ]; then
                break
            fi
            mv $f_name $OUT_PATH/$b
            i=$(( i+1 ))
        done
    done
done
