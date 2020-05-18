#!/bin/bash

# https://www.spec.org/cpu2017/Docs/runcpu-avoidance.html
# https://www.spec.org/cpu2017/Docs/runrules.html#research
# https://www.spec.org/cpu2017/Docs/utility.html#specinvoke
# Run with ref workload after test checks out
# For 627.cam4_s, remember to change process stack size and set environment variables
# Warnings with 502.gcc_r and 602.gcc_s?
# result=$(runcpu --config=$config --size=test --copies=1 --noreportable --iterations=1 ${BENCHMARKS[$i]})

SPEC_DIR="/nfs/cm/scratch1/emery/msteranka/SPEC_CPU2017"
ALLOCATOR_DIR="/nfs/cm/scratch1/emery/msteranka/shared"
LIBENTROPRISE_DIR="/home/msteranka/entroprise/libentroprise"
GLOBAL_OUTPUT_DIR="/nfs/cm/scratch1/emery/msteranka/entroprise-spec"
declare -a ALLOCATORS=('dieharder' 'freeguard' 'guarder' 'hoard' 'jemalloc' 'malloc' 'mesh' 'scudo' 'smimalloc' 'tcmalloc')
declare -a BENCHMARKS=(
    '500.perlbench_r' '502.gcc_r' '505.mcf_r' '520.omnetpp_r' '523.xalancbmk_r'
    '525.x264_r' '531.deepsjeng_r' '541.leela_r' '548.exchange2_r' '557.xz_r' 
    '600.perlbench_s' '602.gcc_s' '605.mcf_s' '620.omnetpp_s' '623.xalancbmk_s' 
    '625.x264_s' '631.deepsjeng_s' '641.leela_s' '648.exchange2_s' '657.xz_s'
    '503.bwaves_r' '507.cactuBSSN_r' '508.namd_r' '510.parest_r' '511.povray_r' 
    '519.lbm_r' '521.wrf_r' '526.blender_r' '527.cam4_r' '538.imagick_r' 
    '544.nab_r' '549.fotonik3d_r' '554.roms_r' '603.bwaves_s' '607.cactuBSSN_s' 
    '619.lbm_s' '621.wrf_s' '627.cam4_s' '628.pop2_s' '638.imagick_s' 
    '644.nab_s' '649.fotonik3d_s' '654.roms_s'
)
NUM_BENCHMARKS=${#BENCHMARKS[@]}

source "$SPEC_DIR/shrc"
for a in "${ALLOCATORS[@]}"
do
    echo "RUNNING $a"
    echo "=================================================="
    for b in "${BENCHMARKS[@]}"
    do
        printf "Running $b..."
        cd "$SPEC_DIR/benchspec/CPU/$b/run/run_base_test_entroprise-m64.0000"
        cmd=$(eval "specinvoke -n | grep -v \# | head -n -1") # Fetch SPEC command to run benchmark -- multiple commands though?
        cmd=${cmd%%>*} # Remove output redirection
        if [ $a = 'malloc' ]
        then
            cmd="LD_PRELOAD=$LIBENTROPRISE_DIR/libentroprise.so $cmd" # Add LD_PRELOAD
        else
            cmd="LD_PRELOAD=$LIBENTROPRISE_DIR/libentroprise.so:$ALLOCATOR_DIR/lib$a.so $cmd" # Add LD_PRELOAD
        fi
        cmd="$cmd > $GLOBAL_OUTPUT_DIR/$a/$b-$a.out" # Redirect entroprise output
        eval "$cmd"
        echo " Done"
    done
    echo ""
done
