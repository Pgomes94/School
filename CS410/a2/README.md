Patrick Gomes
Michael Gerakis
Jason (Zhenjie Ruan)

CS 410 Assignment 2

All group members collaborated and equally divided the work done. 

run 'make' to make all files needed and 'make clean' to clean all files created by the make

Features:
myshell is our command line interpreter which takes commands, parses and then runs them like a normal bash shell. 
tsp_p creates a shared memory for each process and runs the traveling salesman program, writing the paths to stdout
tsp_t creates a thread for each node and runs the traveling salesman program for each thread, writing the paths to stdout
tspsort takes the output from either of the tsp processes and sorts it and writes the sorted paths to stdout

Limitations:
myshell doesn't work properly with inputs that have quotation marks (tested via grep "test" someFile)
