#!/bin/bash

outspath="out" # Directory for output of each allocator
outs=("dieharder" "freeguard" "guarder" "scudo" "jemalloc" "tcmalloc" "mimalloc-secure") # Names for output files
libspath="/root/496/lib" # Directory for shared libraries
libs=("libdiehard.so" "libfreeguard.so" "libguarder.so" "libclang_rt.scudo-x86_64.so" "libjemalloc.so" "libtcmalloc.so.4" "libmimalloc-secure.so") # Names of shared libraries (order of libraries must match names of each in outs)

i=0
while [ $i -lt ${#outs[@]} ]
do
	echo "running ${outs[$i]}"
	LD_PRELOAD="$libspath/${libs[$i]}" ./malloc > "$outspath/${outs[$i]}" # Perform runs+ks for all allocators
 	i=$(( i+1 ))
done
