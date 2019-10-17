# TLDR
DieHarder's entropy stays the same across size classes. FreeGuard and Guarder also show a consistently high entropy, even higher than DieHarder.

# Some Notes
So I ran all of the allocators on the same program (entropy.cpp) and I did so with 1, 2, 4, and 8 threads, 100, 1000, and 10000 objects, and sizes 16B to 64KB (I didn't do 100000 objects because DieHarder just took too long to finish).

I changed the program you gave me to use multiple threads, and I also made it so you would pass the size class, number of objects, and number of threads as arguments. Then I wrote run.sh to run the program with all the allocators (probably not the most efficient way of doing it, but I was concerned that running them together might affect the behavior of the allocators).

When running multiple threads, I took the average entropy of all the threads instead of listing them individually for the sake of readability.

Fun fact: DieHarder actually finished significantly sooner than FreeGuard and Guarder (no idea why).

# DieHarder
I think you can safely say that DieHarder's entropy does not decrease as object size increases. It does seem like it drops a bit when allocating a small number of objects (100 in this case), but when allocating many objects (1000+), it stays pretty consistent. Also, when the entropy does slightly drop, it doesn't drop nearly as much as the Guarder paper claims that it does. Additionally, the entropy seems to stay the same across multiple threads.

# FreeGuard
So this one was kind of surprising because FreeGuard's entropy is consistently higher than DieHarder's, even though FreeGuard only uses 4 freelists to choose objects from. I'm assuming that this is happening because objects are reused in a FIFO order, and with the way entropy is being measured here, it's fairly unlikely to encounter the same object again, even though FreeGuard actually only has 2 bits of entropy. The entropy also stayed the same across multiple threads, and there was consistently lower entropy with 16 byte objects than other size classes.

# Guarder
Guarder seems to show the highest entropy out of the 3 allocators, and it stays quite consistent across multiple threads. Also, when allocating 10000 objects, 16B and 64KB objects had a smaller entropy, and when allocating 1000 objects, 16B objects showed a lower entropy as well. Funny enough, the program actually ran out of memory on the last run with Guarder (8 threads, 10000 objs, 64KB):

7f4fdcfb0740 [FATALERROR]:       ./bibopheap.hh:462 : heap out of memory (bag size = 8589934592)

However, that was only the last run. The rest of the data is fine.
