# Message Cycle
## Description
This is an academic project ilustrating the usage of pipes. Parent process creates 3 children. They are communicating by pipes in a cycle parent -> child1 -> child2 -> child3 -> parent. At first parent sends number 1. Then every process multiplies it by random number. Program ends gracefully when 0 is received by any process.
## Usage
To run this project, use:
```
make
./sample.o
```
