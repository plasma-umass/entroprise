import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import random
import math
from scipy import stats
import statistics
import argparse
import struct
import sys

# Thread 2: 33149

def get_addrs(input_file):
    with open(input_file, 'rb') as f:
        buf = f.read()
    num_addrs = struct.unpack('<I', buf[:4])[0]
    addrs = []
    start = 4 + 65552 # sizeof(int) + sizeof(hll::HyperLogLog)
    end = start + num_addrs * 8
    for i in range(start, end, 8):
        cur = struct.unpack('<Q', buf[i:i+8])[0]
        addrs.append(cur)
    assert (len(addrs) == num_addrs),"len(addrs) != num_addrs"
    return addrs

def shuffle_addrs(a, end):
    n = len(a)
    for i in range(0, end - 2):
        r = random.randint(i, n - 1)
        tmp = a[i]
        a[i] = a[r]
        a[r] = tmp
    return a[:end]

p = argparse.ArgumentParser()
p.add_argument("alloc", type=str, help="allocator") # - GLOBAL
p.add_argument("bench", type=str, help="benchmark") # - GLOBAL
p.add_argument("thread", type=str, help="the thread number")
p.add_argument("path", type=str, help="path to binary file")
args = p.parse_args()

# root = '/home/msteranka/entroprise-parsec-native' - GLOBAL
"""
Benchmark parameters:
    - x_mod
    - y_mod
    - dot opacity
    - dot size
"""
# bench_params = {
    # 'blackscholes': (0,),
    # 'bodytrack': (50000, 100000, 0.05, 5),
    # 'canneal': (2,),
    # 'dedup': (3,),
    # 'facesim': (4,),
    # 'ferret': (5,),
    # 'fluidanimate': (6,),
    # 'freqmine': (7,),
    # 'raytrace': (8,),
    # 'streamcluster': (500, 100000, 0.2, 25),
    # 'swaptions': (20000000, 1000000, 0.01, 0.25),
    # 'vips': (50000, 100000, 0.025, 1),
# }
# params = bench_params[args.bench]
# params = (131072, 0.2, 5)
params = (4096, 0.2, 5)
y_mod = params[0]
opacity = params[1]
dot_size = params[2]
# data_path = root + '/' + args.alloc + '/.' + args.bench + '-' + args.alloc + '-entroprise-data' - GLOBAL
data_path = args.path # - PER-THREAD
plt_title = args.alloc + ' ' + args.bench + ' thread ' + args.thread + ' randomness'
plt_file_name = args.thread + '-' + args.alloc + '-' + args.bench + '.png'
x_label = 'Allocation Time'
y_label = 'Address'
# addrs = get_addrs(data_path)
# 8893 is the minimum number of allocations occuring across all benchmarks of interest (streamcluster, bodytrack, vips, swaptions)
addrs = shuffle_addrs(get_addrs(data_path), 33149)

fig, ax = plt.subplots(figsize=(15,10))
plt.title(plt_title, fontsize=32)
ax.set_xlabel(x_label, fontsize=24, labelpad=15)
ax.set_ylabel(y_label, fontsize=24, labelpad=15)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
x = []
for i in range(0, len(addrs)):
    x.append(i)
y = []
for i in range(0, len(addrs)):
    y.append((addrs[i] / 8) % y_mod)
    # y.append(addrs[i] % y_mod)
plt.xlim((-0.05 * len(addrs), 1.05 * len(addrs)))
plt.ylim((-0.05 * y_mod, 1.05 * y_mod))
# print('len(x) = ' + str(len(x)))
# print('len(y) = ' + str(len(y)))
ax.scatter(x, y, s=dot_size, alpha=opacity)
plt.savefig(plt_file_name, bbox_inches='tight')
# print('Successfully created ' + plt_file_name + ', Number of Allocations: ' + str(len(addrs)))
print('Successfully created ' + plt_file_name)
