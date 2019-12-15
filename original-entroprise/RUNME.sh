#!/bin/bash
# Runs all allocators for every given object size, number of objects, and number of threads

outspath="out" # Directory for output of each allocator
outs=("dieharder" "freeguard" "guarder" "scudo" "jemalloc" "tcmalloc" "smimalloc") # Names for output files
libspath="/root/496/lib" # Directory for shared libraries
libs=("libdiehard.so" "libfreeguard.so" "libguarder.so" "libclang_rt.scudo-x86_64.so" "libjemalloc.so" "libtcmalloc.so.4" "libmimalloc-secure.so") # Names of shared libraries (order of libraries must match names of each in outs)
sizes=(16 32 64 128 256 512 1024 2048 4096 8192 16384 32768) # Size classes
nobjs=(100 1000 10000) # Number of objects
nthreads=(1 2 4 8) # Number of threads

run() { # Runs $1 allocator and puts output in $2 file
	export "LD_PRELOAD=$1"
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
				./entroprise $k $j $i >> $2
			done
			echo "" >> $2
		done
	done
}

make # Compile entroprise

for i in "${outs[@]}"
do
	echo > "$outspath/$i" # Clear out files
done

n=0
while [ $n -lt ${#outs[@]} ]
do
	echo "running ${outs[$n]}"
	run "$libspath/${libs[$n]}" "$outspath/${outs[$n]}" # Run entroprise with all allocators
 	n=$(( n+1 ))
done
