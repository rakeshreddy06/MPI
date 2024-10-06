# MPI Sorting and Stencil Computation

This project contains MPI implementations for sorting and stencil computation.

## Setup

1. Update and upgrade your system:
sudo apt-get update
sudo apt-get upgrade


2. Install OpenMPI:
sudo apt-get install libopenmpi-dev
sudo apt-get install openmpi-bin


3. Verify installation:
mpicc --version
mpiexec --version


## Compilation and Execution

### Sorting Program

1. Compile:
mpicc sort_all.c -o sortout


2. Execute:
mpiexec --oversubscribe -np 4 ./sortout


### Stencil Computation

1. Compile:
mpicc stencil_all.c -o stencilout


2. Execute:
mpiexec --oversubscribe -np 1 stencilout 100 10 50 1 1


## Note

The `--oversubscribe` option allows more processes than available slots in Open MPI.
