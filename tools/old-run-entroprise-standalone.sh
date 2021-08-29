# !/bin/bash
# Calculates entropy on a given set of allocators for a given set of object sizes, number of objects, and number of threads

# Names of allocators
#
# NOTE: The shared libraries corresponding to all allocators must be named in the following format:
# lib<ALLOCATOR_NAME>.so
# Otherwise, LD_PRELOAD will fail to find the correct shared library
libs=("dieharder" "freeguard" "guarder" "scudo" "jemalloc" "mimalloc" "mesh" "hoard" "tcmalloc" "smimalloc")
libspath="/nfs/cm/scratch1/emery/msteranka/shared" # Directory containing shared libraries for allocators
outpath="/home/msteranka/entroprise/data" # Directory for output files
outpostfix="-entropy.out"
# libs=("libdiehard.so" "libfreeguard.so" "libguarder.so" "libclang_rt.scudo-x86_64.so" "libjemalloc.so" "libtcmalloc.so.4" "libmimalloc-secure.so") # Names of shared libraries (order of libraries must match names of each in outs)
# sizes=(16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536) # Size classes
sizes=(16 32 64) # Size classes
nobjs=(1000 10000 100000) # Number of objects
nthreads=(1 2) # Number of threads

run() { # Runs $1 allocator and puts output in $2 file
	# export "LD_PRELOAD=$1"
	for i in "${nthreads[@]}"
	do
		echo "NUM_THREADS: $i" >> $2
		for j in "${nobjs[@]}"
		do
			echo -e "\tNUM_OBJECTS: $j" >> $2
			for k in "${sizes[@]}" 
			do
				printf "\t\t%-5d   " $k >> $2
				# LD_PRELOAD=$1 ./entroprise $k $j $i >> $2
				./entroprise $k $j $i y >> $2
			done
			echo "" >> $2
		done
	done
}

for i in "${libs[@]}"
do
	echo "" > "$outpath/$i$outpostfix" # Empty output files
done

n=0
while [ $n -lt ${#libs[@]} ]
do
	# echo "Running ${libs[$n]}"
	# run "$libspath/${libs[$n]}" "$outspath/${outs[$n]}" # Run entroprise with all allocators
	for i in "${nthreads[@]}"
	do
		echo "NUM_THREADS: $i" >> $2
		for j in "${nobjs[@]}"
		do
			echo -e "\tNUM_OBJECTS: $j" >> $2
			for k in "${sizes[@]}" 
			do
				printf "\t\t%-5d   " $k >> $2
				# LD_PRELOAD=$1 ./entroprise $k $j $i >> $2
				./entroprise $k $j $i y >> $2
			done
			echo "" >> $2
		done
	done
 	n=$(( n+1 ))
done
