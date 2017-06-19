# Tarea 2
## Tópicos Avanzados de Sistemas Distribuidos

Student: Giovanni De Lucca

## Main Instruction
Compile code.
```sh
make
```
Run the application.
```sh
mpirun -n <N> --hostfile <hostnames> main <input_file> 
```
N: N° of nodes

hostnames: File with the address of nodes.

input_file: File with the graph data

## Test Instruction
Compile code.
```sh
make checktest
```
Run tests.
```sh
mpirun -n <N> --hostfile <hostnames> checktest
```
N: N° of nodes

hostnames: File with the address of nodes.