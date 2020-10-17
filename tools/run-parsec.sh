#!/bin/bash

# Changes to $PARSEC_DIR/bin/parsecmgmt:
#   - Comment out lines 606 to 620 to prevent checking validity of submit option

PARSEC_DIR="/nfs/cm/scratch1/emery/msteranka/parsec-3.0"
ALLOCATOR_DIR="/nfs/cm/scratch1/emery/msteranka/shared"
LIBENTROPRISE_DIR="/home/msteranka/entroprise/libentroprise"
GLOBAL_OUTPUT_DIR="/nfs/cm/scratch1/emery/msteranka/entroprise-parsec"
declare -a ALLOCATORS=('dieharder' 'freeguard' 'guarder' 'hoard' 'jemalloc' 'malloc' 'mesh' 'scudo' 'smimalloc' 'tcmalloc')
declare -a BENCHMARKS=('blackscholes' 'bodytrack' 'canneal' 'dedup' 'facesim' 'ferret' 'fluidanimate' 'freqmine' 'raytrace' 'streamcluster' 'swaptions' 'vips' 'x264')
declare -a CONFIGS=('gcc' 'gnu++98-config' 'gcc' 'gcc' 'gnu++98-config' 'gcc' 'gnu++98-config' 'gcc' 'gcc' 'gcc' 'gnu++98-config' 'gcc' 'no-pie-config')
NUM_BENCHMARKS=${#BENCHMARKS[@]}

source "$PARSEC_DIR/env.sh"
for a in "${ALLOCATORS[@]}"
do
    echo "RUNNING $a"
    echo "=================================================="
    for (( i=0; i<${NUM_BENCHMARKS}; i++ ));
    do
        printf "Running ${BENCHMARKS[$i]}..."
        if [ $a = 'malloc' ]
        then
            result=$(parsecmgmt -a run -p "${BENCHMARKS[$i]}" -c "${CONFIGS[$i]}" -s "LD_PRELOAD=$LIBENTROPRISE_DIR/libentroprise.so" -i test -n 1)
        elif [ $a = 'scudo' ]
        then
            result=$(parsecmgmt -a run -p "${BENCHMARKS[$i]}" -c "${CONFIGS[$i]}" -s "LD_PRELOAD=$LIBENTROPRISE_DIR/libentroprise.so:$ALLOCATOR_DIR/lib$a.so SCUDO_OPTIONS=DeallocationTypeMismatch=0" -i test -n 1)
        else
            result=$(parsecmgmt -a run -p "${BENCHMARKS[$i]}" -c "${CONFIGS[$i]}" -s "LD_PRELOAD=$LIBENTROPRISE_DIR/libentroprise.so:$ALLOCATOR_DIR/lib$a.so" -i test -n 1)
        fi
        echo "$result" > "$GLOBAL_OUTPUT_DIR/$a/${BENCHMARKS[$i]}-$a.out"
        echo "Done"
    done
    echo ""
done
