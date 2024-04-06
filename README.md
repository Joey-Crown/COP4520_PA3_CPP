# Build and Execute

This program was made using C++ and CMake. Both problems will run and output the time taken for each to complete. 
To build  this command in the project directory:
```
cmake --build ./cmake-build-debug --target COP4520_PA3_CPP -j 9
```
And Run the program with:
```
./cmake-build-debug/COP4520_PA3_CPP
```

I have some last minute bugs that I think might be due to the way I handle random number generation. As I'll explain I 
wasted a lot of time on this project thinking I could do it in Rust, which turned out to be more trouble than it's worth.

# Problem 1: The Birthday Presents Party

I originally tried to complete this problem in Rust but after some difficulties with trying to implement a doubly-linked
list I learned that this is a classically difficult thing to do correctly in Rust, especially when trying to make it
thread-safe. The Rust borrow-checker does not like iterating through non-contiguous memory, as it's designed to drop any
piece of memory when there is no variables referencing it.

Eventually, I gave up and decided to do it in C++ using a coarse-grained lock approach, with some modifications to reduce
constant freeing of memory. During list removals, nodes which are to be removed are instead marked as logically deleted.
This means when iterating through the list to check if an item is contained in it, the list iterator will move to the 
next node without checking the value of the deleted node. 

I created a non-uniform random distribution of the chance that a servant does a particular chance. Insertions, removals,
and checks are all equally likely, but there is a 1/25 chance a thread will execute a "cleanup" operation, where it iterates
through the list and does a real removal of all nodes marked as logically deleted.

The program runs and threads write cards after a logical deletion task, and Problem 1 finishes execution after all gifts
have cards written for them.

Problem 1 takes about 60 seconds on my system utilizing 8 threads. From my research a lock-free approach would be more
performant but much more challenging to implement well. Also, my console outputs significantly reduce performance

# Problem 2: Atmospheric Temperature Reading Module

This program is pretty simple, we just have to make an array with capacity to hold every "minute" of the hour the 
program executes for each thread. Before creating a report we check that all sensors have a reading and if they, 
proceed with locking the critical section where we calculate the info necessary to generate the report.

Runtime is dependent on how long each "minute" should be simulated, and as long as the program executes normally, there
shouldn't be any performance concerns
