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
    return addrs

p = argparse.ArgumentParser()
p.add_argument("alloc", type=str, help="allocator")
p.add_argument("bench", type=str, help="benchmark")
args = p.parse_args()

root = '/home/msteranka/entroprise-parsec-native'
"""
Benchmark parameters:
    - x_mod
    - y_mod
    - dot opacity
    - dot size
"""
bench_params = {
    # 'blackscholes': (0,),
    'bodytrack': (1000, 1000000, 0.2, 10),
    # 'canneal': (2,),
    # 'dedup': (3,),
    # 'facesim': (4,),
    # 'ferret': (5,),
    # 'fluidanimate': (6,),
    # 'freqmine': (7,),
    # 'raytrace': (8,),
    'streamcluster': (500, 100000, 0.2, 25),
    'swaptions': (10,),
    'vips': (11,),
}
params = bench_params[args.bench]
x_mod = params[0]
y_mod = params[1]
opacity = params[2]
dot_size = params[3]
data_path = root + '/' + args.alloc + '/.' + args.bench + '-' + args.alloc + '-entroprise-data'
plt_title = args.alloc + ' ' + args.bench + ' randomness'
plt_file_name = args.alloc + '-' + args.bench + '.png'
x_label = 'Iteration'
y_label = 'Address'
addrs = get_addrs(data_path)

fig, ax = plt.subplots(figsize=(15,10))
plt.title(plt_title, fontsize=32)
ax.set_xlabel(x_label, fontsize=24, labelpad=15)
ax.set_ylabel(y_label, fontsize=24, labelpad=15)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
x = []
for i in range(0, len(addrs)):
    x.append(i % x_mod)
y = []
for i in range(0, len(addrs)):
    y.append(addrs[i] % y_mod)
plt.xlim((-0.05 * x_mod, 1.05 * x_mod))
plt.ylim((-0.05 * y_mod, 1.05 * y_mod))
ax.scatter(x, y, s=dot_size, alpha=opacity)
plt.savefig(plt_file_name, bbox_inches='tight')
print('Successfully created ' + plt_file_name)
