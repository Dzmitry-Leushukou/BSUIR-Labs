# Lab 8: Maximum Flow

The program implements the Ford-Fulkerson algorithm using the labeling method
(breadth-first search) to find augmenting paths. Capacities are nonnegative
integers. As in the assignment, every arc must have its reverse arc in the
input, including reverse arcs with zero capacity. The residual capacity is
calculated as `c(u,v) - f(u,v) + f(v,u)` and flow is updated by the formula in
the assignment.

## Build and run

```sh
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o lab8
./lab8 input.txt
```

Input format:

```text
vertex_count arc_count source sink
from_1 to_1 capacity_1
...
from_m to_m capacity_m
```

Vertices are numbered from 1 to `vertex_count`. The program reports each
augmenting path and its bottleneck, the final flow on every input arc, the
maximum-flow value, and the source side of a minimum cut.

## Sample

`input.txt` is the example network from the assignment, with reverse arcs
listed explicitly: maximum flow is 3 and the minimum cut is `{1, 2, 3} | {4}`.
