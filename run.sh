#!/bin/bash

path="LD_PRELOAD=/root/496/lib" # Directory for libraries

run() {
	sizes=(16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536)
	nobjs=(100 1000 10000)
	nthreads=(1 2 4 8)

	export $1
	for i in "${nthreads[@]}"
	do
		echo "NTHREADS: $i" >> $2
		for j in "${nobjs[@]}"
		do
			echo -e "\tNOBJS: $j" >> $2
			for k in "${sizes[@]}" 
			do
				printf "\t\t%-5d   " $k >> $2
				./a.out $k $j $i >> $2
			done
			echo "" >> $2
		done
	done
}

clang++ -std=c++11 -pthread entroprise.cpp
rm dieharder.txt freeguard.txt guarder.txt
echo "Running DieHarder..."
run $path/libdiehard.so dieharder.txt
echo "Running FreeGuard..."
run $path/libfreeguard.so freeguard.txt
echo "Running Guarder..."
run $path/libguarder.so guarder.txt
