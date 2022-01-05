Corresponding Paper:
Chuan Luo, Shaowei Cai, Kaile Su, Wenxuan Huang. CCEHC: An Efficient Local Search Algorithm for Weighted Partial Maximum Satisfiability. Artificial Intelligence 243: 26-44 (2017).

Notice: The operating system for compiling and running the CCEHC solver should be 64-bit GNU/Linux.

For compiling:
make

For running:
./CCEHC -inst <instance> -seed <seed> -t <cutoff_time> -p <prob> -sp <smooth_probability>

For solving random instances:
./CCEHC -inst <instance> -seed <seed> -t <cutoff_time> -p 0.2 -sp 0.0001

For solving crafted instances:
./CCEHC -inst <instance> -seed <seed> -t <cutoff_time> -p 0.177 -sp 0.003

For solving application instances:
./CCEHC -inst <instance> -seed <seed> -t <cutoff_time> -p 0.279 -sp 0.085
