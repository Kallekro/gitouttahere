# Guide to the performance simulator

## The model

The performance simulator has a configurable timing model of a modern computer.
By default it models a 9 stage pipeline corresponding to the "realistic" pipeline
introduced at the lectures, but it can be scaled up or down through options given
below.

It has a 3 level memory hierarchy:

* L1 separate instruction and data caches, 16KB each, 3-cycle fully pipelined access
* L2 shared cache, 256KB, 9 cycle fully pipelined access
* Main memory, 100 cycles

Fully pipelined access means that even though it takes multiple cycles, it can
start a new access every cycle.

The model is not completely accurate. Here are some known limitations:

* The effect of non-completed/aborted instructions are not modelled.
  Even though such instructions may not cause visible changes to memory
  or registers, they may have performance effects.

* Structural hazards may not be modelled correcly. Prime example is
  a miss in both data and instruction cache at the same time, but from
  different instructions. Since modelling is done in instruction
  order, though an instruction miss from a later instruction may occur
  before a data cache miss from an earlier instruction, the simulator
  will not know that. In real life the instruction cache miss may get
  access to the L2 cache first, thus delaying the data cache miss access to L2,
  but in the simulator the effect will be reversed.

* The modelling of instruction fetch is only partially meaningfull.
  To detect an instructin you need to first fetch and then decode it, so
  in real life, instruction fetch is done for a cache block, without
  knowing if it will contain one or multiple instructions. The simulator
  focuses on introducing a correct delay from a control flow change is
  decided, to the target instruction can finish decoding.

* The modelling of main memory is much simpler than reality.

In summary, it's a "best effort" that should be accurate to within a few
percent. Please report obvious errors.



## Understanding the summary output

If you give option '-m' you get a summary output after simulation:

~~~
Caches and predictors:

    Branch predictions: 127550   hits: 124999   hitrate: 0.980000
    Return predictions: 1   hits: 1   hitrate: 1.000000
    Cache: ICache
        Reads: 2147709,    Writes 0
        Hits: 2147704,    Hitrate: 0.999998
        Average latency: 3.000750
    Cache: DCache
        Reads: 375002,    Writes 127501
        Hits: 474766,    Hitrate: 0.944802
        Average latency: 3.830025
    Cache: L2-Cache
        Reads: 27742,    Writes 0
        Hits: 27110,    Hitrate: 0.977219
        Average latency: 11.278134

Timing info:

    Instructions: 2147709   cycles (last value): 4853009
    IPC: 0.442552   CPI: 2.259621
~~~

IPC means "instructions per cycle", CPI means "cycle per instruction".


## Understanding the pipeline model

Instructions pass through different *states* during execution. Some of
these states correspond to *stages* in the pipeline. Others do not. It
can happen that instructions pass through multiple states in a single
pipeline stage in a single cycle.

The states tracked by the model are:

* 'F' - Start of instruction fetch / instruction cache access.
* 'D' - Start of instruction decode.
* 'q' - Queued for operands (decode is done, instruction must wait)
* 'r' - Ready. Operands available, waits for execution resource
* 'X' - Start of execute for an arithmetic operation
* 'A' - Address generate start (similar to 'X' but for memory accesses)
* 'M' - Multiply start (similar to 'X' but for multiply)
* 'J' - Make control flow decision (similar to 'X' but for jmp, call, ret)
* 'L' - Start of data cache access for instructions that read from memory
* 'S' - Start of data cache access for instructions that write to memory
* 'w' - Instruction has it's result ready. ("writeback")
* 'C' - The effects of the instruction is fully "committed", can not abort.

The 'q' and 'r' states are often passed through immediately, effectively
hidden inside the 'D' or 'X' (or 'A' or 'J' or 'M') states, but they are useful
for understanding the source of delays in an advanced pipeline.

The following is a (simplified) part of the output you can get (use '-c'). To the
right there is a pipeline diagram illustrating the flow of the instructions and time:
(instructions flow down, time flows from left to right)

~~~
48   addq %rdx, %r11           : %r11 = 26         |   F..Dq..|XwC       |       
4a   mulq %r15, %r11           : %r11 = 130        |    F.....|DM...wC   |       
4c   movq 1032(%r11), %r12     : %r12 = 201A       |     F....|.Dq..AL..w|C
52   movq %rdx, %r11           : %r11 = 26         |      F...|.....DXw..|.C
~~~

You can see that fetch commences with one instruction per clock, and for the
first instruction you can see the 3-cycle latency before decode starts. Decode is
done in a single cycle, but the 'q' indicates that the instruction cannot
proceed because it lacks operands. So either %rdx or %r11 is being written
by a prior instruction which has't finished yet.

The instruction waits for operands for 3 cycles, then transitions through the 'r'
state with no delay (so you can't see it) and into the 'X' state. It is a simple
arithmetich instruction so execution completes in one clock. Then comes writeback 
('w') and commit 'C'.

You can see the effect on the following instruction, which is stalled and cannot
enter 'D'-state before the previous instruction enters 'X'.

The instruction at 4a is a multiply instruction. The multiplier is fully pipelined
and has a latency of four cycles (which is normal for a multiplier), before it
produces a result.

This delays the next instruction at 4c, which must wait for the result of the multiply
before it can start address generation. Note that the model assumes a forwarding path
from the end of the multiplier to the start of the address generator - otherwise the
instruction would have had to wait even more.

## Understanding the profile output

You can get an execution profile by specifying "-prof". Specifying "-m" along with
"-prof" provides details of how long each instruction has been in specific states.

Here is a partial result from specifying just "-prof":

~~~
addr : count  : imiss dmiss pmiss : disassembly
  70 : 125000 :                   : addq %r8, %rdx                
  72 : 125000 :                   : cmpq %rdx, %rax               
  74 : 125000 :               2.0 : jne $0x00000044               
  79 :   2500 :                   : addq %r8, %rcx                
  7b :   2500 :                   : cmpq %rcx, %rax               
  7d :   2500 :               2.0 : jne $0x00000030               
  82 :     50 :   2.0             : addq %r8, %rbx                
  84 :     50 :                   : cmpq %rbx, %rax               
  86 :     50 :               2.0 : jne $0x0000002A               
  8b :      1 :       100.0       : ret                           
~~~

Explanation:

* addr: The address of the instruction
* count: The number of times that instruction was executed
* imiss: The percentage of instruction cache miss
* dmiss: The percentage of data cache miss
* pmiss: The percentage of predictor miss

The percentages refer to the execution count of the particular instruction,
not the total amount of instructions. For example, the return instruction
at address 8b has a 100% dcache miss rate, but that may not mean much, since
it is only executed once.


A more detailed profile is obtained by "-prof -m":

~~~
  60 : 125000 :   0.0             : FDqrMwC    9   1   5   0   4   1         : mulq %r12, %r13               
  62 : 125000 :         0.0       : FDqrALwC  11   1   0   0   1   3   1     : movq 1048(%r10), %r12         
  68 : 125000 :                   : FDqrXwC   11   1   3   0   1   1         : addq %r12, %r13               
  6a : 125000 :                   : FDqrASC   11   1   0   0   1   4         : movq %r13, 1048(%r10)         
  70 : 125000 :                   : FDqrXwC    6   1   0   0   1   4         : addq %r8, %rdx                
~~~

In the detailed printout, the states that an instruction passes through is given
for each instruction next to the number of cycles that instruction spent in a
particular state. Typically the 'q' and 'r' states are of interest, since they
indicate a wait for operands (data dependencies) or resources. In the above
example, the multiply at address 60 spends 5 cycles in the 'q' state waiting
for one or both operands. The add at 68 waits 3 cycles, clearly for the result
of the preceeding instruction which fetches data from memory.

The time spent in each state is averaged over all the runs of each instruction.

The time spent in the 'C' state is always 1 and is omitted from the display.


## Command line arguments, somewhat explained

~~~
Usage: sim input-file [options*]
~~~

Always give file name before other options!



### Modelling options

These options affect the level of detail in which information is collected and
presented.

~~~
  -prof                 collect and display profiling information for each instruction
  -t                    trace: print disassembly and result of each instruction
  -m                    model resource use and estimate timing
  -c                    print cycle diagram for each instruction (implies -m and -t)
  -ddep-only            model only data dependencies, ignore actual resource needs
  -diff                 end simulation by printing registers and changed memory cells
~~~



### Pipeline delays

These options sets the latency for different activities in the pipeline

~~~
  -dlat=<cycles>        latency of decoder (default 1, 2 or 3 dependent on other options)
  -clat=<cycles>        latency of cache read (default 3)
  -l2lat=<cycles>       additional latency of L2 (shared) cache access (default 9)
  -mlat=<cycle>         additional latency of main memory access (default 100)
  -ptlat=<cycles>       latency of prediction if flow change (default 2)
  -pntlat=<cycles>      latency of prediction if no flow change (default 0)
~~~


### Cache configuration

These options configure the 3 caches. A cache is organized as a set of
blocks, each block 1<<blksz long. Default is blksz=5 which corresponds to
a 32-byte block. When a cache miss occurs, the entire block is fecthed
from the underlying layer of the memory hiearchy.

The caches are way-associative, the associativity can be given by the
assoc parameter. Default is 4-way.

The number of blocks in one "way" of the cache is 1<<idxsz. Default for
primary caches is 7, which gives 128 blocks per way. Hence, the default
cache is (1<<5)*(1<<7)*4 = 16KByte in size.

The secondary cache has the same blocksize and associativity but with
(1<<11) blocks per way the total size is 256KByte.

~~~
  -ic:blksz=<bits>      select number of bits for indexing i-cache block (default 5)
  -ic:idxsz=<bits>      select number of bits in i-cache index (default 7)
  -ic:assoc=<num>       select i-cache associativity (default 4)
  -dc:blksz=<bits>      select number of bits for indexing d-cache block (default 5)
  -dc:idxsz=<bits>      select number of bits in d-cache index (default 7)
  -dc:assoc=<num>       select d-cache associativity (default 4)
  -l2:blksz=<bits>      select number of bits for indexing L2-cache block (default 5)
  -l2:idxsz=<bits>      select number of bits in L2-cache index (default 11)
  -l2:assoc=<num>       select L2-cache associativity (default 4)
~~~


### Predictor configuration

The model includes 3 predictors:

 * A return predictor with just 4 entries (not configurable)

 * A branch target cache, which is only indirectly modelled through
   the latencies for predicting a taken/non-taken branch.
   The default setting is 0 cycles latency for predicting not-taken
   and 2 cycles latency for predicting taken.

 * A configurable branch direction predictor. You can choose among:

   * oracle: Predicts everything correct
   * taken: Predicts every branch taken
   * nt: Predicts every branch not taken
   * btfnt: Predicts backward branches taken, forward branches not taken
   * local: A dynamic predictor based on past history for each branch in 
     isolation. You can select the number of bits used to index the
     history table.
   * gshare: A dynamic predictor based on the combined history 
     of the last branches as well as the branch in isolation. You can
     select the number of bits used to index the history table.

 * The default branch predictor is "gshare" with a 12-bit index (4K table).


~~~
  -bpred=oracle         select oracle predictor (no mispredictions)
  -bpred=taken          select taken predictor
  -bpred=nt             select not taken predictor
  -bpred=btfnt          select backward-taken, forward-not-taken predictor
  -bpred=gshare:<size>  select gshare predictor of given size (default 12 bits)
  -bpred=local:<size>   select local predictor of given size
~~~


### Superscalar resource configuration

These options increase the width of the pipeline (-pw) or selected
execution resources within the pipeline.

Increasing the width of the pipeline (going superscalar) automatically
increases the latency of the decode stage with one cycle.

~~~
  -pw=<width>           width of pipeline (default 1)
  -cp=<ports>           number of cache ports (default: 1)
  -xp=<ports>           number of agen/alu ports (=units) (default: half of -pw +1)
  -mp=<ports>           number of multiplier ports (=units) (default: 1)
~~~


### Out-of-order pipeline configuration

These options enable dynamic out-of-order scheduling. Enabling out-of-order
scheduling automatically increases the latency of the decode stage with
one cycle.

The size of the reorder buffer (-rob=size) limits the total number
of instructions the pipeline can hold from decode to commit.

The size of the cache queue and execute queue limits the number
of instructions which can be held waiting for respectively the
cache and the arithmetic units, while other later instructions
are allowed to execute.

~~~
  -ooo                  enable out-of-order scheduling
  -rob=<size>           give size of reorder buffer (default 128)
  -cq=<size>            give size of queue/scheduler for cache access (default 32)
  -xq=<size>            give size of queue/scheduler for execute stage (default 64)
~~~
