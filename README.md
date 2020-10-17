# Entroprise

Entroprise is a utility for evaluating the object reuse security of a memory allocator by analyzing the entropy and randomness of address sequences. Entropy is efficiently measured using a HyperLogLog data structure, and randomness is calculated by combining the Wald-Wolfowitz runs test and Kolmogorov-Smirnov test. 

## Usage

Entroprise comes in two forms: entroprise and entroprise-standalone. The former can be run on any given program while the latter is essentially a micro benchmark that repeatedly calls malloc and free. Both entroprise and entroprise-standalone can be compiled with the following commands:

    $ cd src/
    $ make

To run a program using entroprise with the default system allocator, run the following:

    $ ./entroprise /path/to/program <program_arguments>

To run it with a given allocator:

    $ ./entroprise -a /path/to/allocator.so /path/to/program <program_arguments>

The output of entroprise will display the entropy and randomness corresponding to each thread as well as a summary outlining all global data. One can also run entroprise-standalone to determine an allocator's effectiveness in thwarting the worst-case scenario of repeatedly calling malloc and free:

    $ ./entroprise-standalone 32 100000 8 y

The above command will spawn 8 threads, each of which will allocate 100000 objects of 32 bytes. Additionally, entroprise-standalone will approximate the entropy rather than calculate it directly with this command, as indicated by the last argument.
