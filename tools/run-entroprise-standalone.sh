# !/bin/bash
# Calculates entropy on a given set of allocators for a given set of object sizes, number of objects, and number of threads

# Names of allocators
#
# NOTE: The shared libraries corresponding to all allocators must be named in the following format:
# lib<ALLOCATOR_NAME>.so
# Otherwise, LD_PRELOAD will fail to find the correct shared library
# libs=("freeguard" "guarder" "scudo" "jemalloc" "mimalloc" "mesh" "hoard" "tcmalloc" "smimalloc" "dieharder")
libs=("dieharder")
libspath="/nfs/cm/scratch1/emery/msteranka/shared" # Directory containing shared libraries for allocators
outpath="../data/tmp" # Directory for output files
entroprisepath="../src" # Path to entroprise-standalone
outpostfix="-entropy.out"
bitwidth="16"
sizes=(16 32 64) # Size classes
# sizes=(16 32 64 128 256) # Size classes
nobjs=(10000 100000 1000000) # Number of objects
# nobjs=(10000 100000 1000000 10000000) # Number of objects
# nobjs=(10000) # Number of objects
nthreads=(1 2 4 8) # Number of threads

for i in "${libs[@]}"
do
	echo -n "" > "$outpath/$i$outpostfix" # Empty output files
done

for l in "${libs[@]}"
do
    echo "ALLOCATOR: $libspath/lib$l.so" >> $outpath/$l$outpostfix
    echo -e "BIT_WIDTH: $bitwidth\n" >> $outpath/$l$outpostfix
	for i in "${nthreads[@]}"
	do
		for j in "${nobjs[@]}"
		do
			for k in "${sizes[@]}" 
			do
                # TODO: When changing sizes/nobjs/nthreads, remember to also change the formatting
                printf "THREADS: %-2d     NUM_ALLOCS: %-8d     ALLOC_SIZE: %-4d     ENTROPY: " $i $j $k >> $outpath/$l$outpostfix
                echo "LD_PRELOAD=$libspath/lib$l.so $entroprisepath/entroprise-standalone $k $j $i y $bitwidth >> $outpath/$l$outpostfix"
                # LD_PRELOAD=$libspath/lib$l.so /usr/bin/time -f ", ELAPSED_SECONDS=%es" $entroprisepath/entroprise-standalone $k $j $i y $bitwidth >> $outpath/$l$outpostfix
                LD_PRELOAD=$libspath/lib$l.so /usr/bin/time -f "     ELAPSED_SECONDS=%es" $entroprisepath/entroprise-standalone $k $j $i y $bitwidth >> $outpath/$l$outpostfix 2>> $outpath/$l$outpostfix
                # /usr/bin/time -f "     ELAPSED_SECONDS=%es" $entroprisepath/entroprise-standalone $k $j $i y $bitwidth >> $outpath/$l$outpostfix 2>> $outpath/$l$outpostfix
                # LD_PRELOAD="$libspath/libmarkusgc.so $libspath/libmarkusgccpp.so" /usr/bin/time -f "     ELAPSED_SECONDS=%es" $entroprisepath/entroprise-standalone $k $j $i y $bitwidth >> $outpath/$l$outpostfix 2>> $outpath/$l$outpostfix
			done
		done
	done
done

