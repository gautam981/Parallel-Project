## Project
Parallel DAG-Based Task Scheduling with Dynamic Load Balancing using Work-Stealing in Shared Memory Systems.

## Files
| File | Description |
|------|-------------|
| `dag.h` | Task and DAG data structures |
| `thread_pool.h/.cpp` | Work-stealing thread pool |
| `scheduler.h/.cpp` | DAG-aware parallel scheduler |
| `main.cpp` | Benchmark driver (sequential vs parallel) |
| `Makefile` | Build configuration |

## Build & Run
```bash
make
./dag_scheduler
```
Or simply:
```bash
make run
```

## How It Works
1. Tasks and their dependencies are represented as a DAG
2. Tasks with no unsatisfied dependencies are dispatched to worker thread queues
3. Each thread has its own lock-based deque (push/pop from front)
4. Idle threads steal tasks from the back of other threads' deques
5. When a task completes, it decrements successor dependency counters
6. Successors with zero remaining dependencies are immediately dispatched

## Requirements
- g++ with C++17 support
- Linux (pthread)
- `make`
3. "Is scheduling ko implement karne ke baad humne apne test mein almost **5.4x ka speedup** achieve kiya jab humne apne threads badhaye, jo humare benchmark graph mein cleanly dikhta hai."
