# !/bin/bash

# run-entroprise-standalone.sh
#
# Calculates the entropy and runtime on a given set of allocators, allocation sizes, 
# numbers of allocations, and numbers of threads.

# List of allocators:
#
# DieHarder
# FreeGuard
# Guarder
# Scudo
# smimalloc
# ptmalloc
# jemalloc
# tcmalloc
# mimalloc
# Hoard
# Mesh

# NOTE: The libraries for all allocators must adhere to the following naming convention:
#
# lib<allocator>.so
#
# Otherwise, LD_PRELOAD will fail to find the correct file.

# NOTE: When changing the number of threads, the number of allocations, or the allocation 
# sizes, you should also verify that the formatting of printf(1) will be correct.

allocators=("dieharder")                              # Allocators to run
num_threads=(1 2 4 8)                                 # Number of threads
num_allocs=(1000 10000 100000 1000000 10000000)       # Number of objects
alloc_sizes=(16 32 64 128 256)                        # Size classes
bit_width="16"                                        # Bit width (a larger bit width will yield higher accuracy)

allocators_path="/home/msteranka/allocators"          # Directory containing .so files for allocators
entroprise_path="../src"                              # Path to entroprise-standalone
output_path="../data/tmp/tesla"                       # Directory for output files
output_postfix=".out"
time_path="/usr/bin"

for i in "${allocators[@]}"
do
	echo -n "" > "$output_path/$i$output_postfix"     # Empty existing output files
done

for l in "${allocators[@]}"
do
    echo "ALLOCATOR: $allocators_path/lib$l.so" >> $output_path/$l$output_postfix
    echo -e "BIT_WIDTH: $bit_width\n" >> $output_path/$l$output_postfix
	for i in "${num_threads[@]}"
	do
		for j in "${num_allocs[@]}"
		do
			for k in "${alloc_sizes[@]}" 
			do
                printf "THREADS: %-2d     NUM_ALLOCS: %-8d     ALLOC_SIZE: %-4d     ENTROPY: " $i $j $k >> $output_path/$l$output_postfix
                echo "LD_PRELOAD=$allocators_path/lib$l.so $time_path/time $entroprise_path/entroprise-standalone $k $j $i y $bit_width >> $output_path/$l$output_postfix"
                # LD_PRELOAD=$allocators_path/lib$l.so $entroprise_path/entroprise-standalone $k $j $i y $bit_width >> $output_path/$l$output_postfix
                LD_PRELOAD=$allocators_path/lib$l.so $time_path/time -f "     ELAPSED_SECONDS=%es" $entroprise_path/entroprise-standalone $k $j $i y $bit_width >> $output_path/$l$output_postfix 2>> $output_path/$l$output_postfix
                # $time_path/time -f "     ELAPSED_SECONDS=%es" $entroprise_path/entroprise-standalone $k $j $i y $bit_width >> $output_path/$l$output_postfix 2>> $output_path/$l$output_postfix
                # LD_PRELOAD="$allocators_path/libmarkusgc.so $allocators_path/libmarkusgccpp.so" $time_path/time -f "     ELAPSED_SECONDS=%es" $entroprise_path/entroprise-standalone $k $j $i y $bit_width >> $output_path/$l$output_postfix 2>> $output_path/$l$output_postfix
			done
		done
	done
done

