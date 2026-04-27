#include "scheduler.h"
#include <thread>
#include <iostream>
#include <chrono>

Scheduler::Scheduler(DAG &d, int threads)
    : dag(d), pool(threads), remaining(d.tasks.size()) {}

/**
 * @brief Simulates computationally heavy execution for a task.
 */
void Scheduler::executeTask(std::shared_ptr<Task> /* task */) {
    volatile long long sum = 0;
    for (int i = 0; i < 1000000; i++) { // 1M loop to match sequential baseline
        sum += i;
    }
}

/**
 * @brief Worker thread loop. Continually pops or steals tasks until all tasks in DAG are done.
 */
void Scheduler::worker(int tid) {
    while (true) {
        if (remaining.load() == 0) break; // All DAG tasks have completed

        // 1. Try to pop from local queue
        auto task = pool.popTask(tid);
        
        // 2. If local queue empty, try to steal from others
        if (!task) task = pool.stealTask(tid);

        // 3. If no tasks available to steal, sleep to save CPU power
        if (!task) {
            std::unique_lock<std::mutex> lk(pool.cv_m);
            // We sleep using wait_for (1 ms max) to prevent rare race condition deadlocks 
            // when the final tasks are pushed right as remaining hits 0.
            pool.cv.wait_for(lk, std::chrono::milliseconds(1));
            continue;
        }

        // Execute task work
        executeTask(task);

        // Decrement dependency counters for all dependent tasks
        for (int v : task->dependents) {
            // fetch_sub returns the value BEFORE subtraction. 
            // If it was 1, it becomes 0, meaning dependencies are met.
            if (dag.tasks[v]->dependency_count.fetch_sub(1) == 1) {
                // Task is ready, push to a target thread queue (round-robin)
                pool.pushTask(v % pool.num_threads, dag.tasks[v]);
            }
        }

        // Safely decrement the total remaining tasks counter
        remaining--;
    }
}

/**
 * @brief Initiates the parallel execution engine.
 */
void Scheduler::run() {
    // Seed initial ready tasks (those with 0 dependencies)
    for (auto &t : dag.tasks) {
        if (t->dependency_count.load() == 0) {
            pool.pushTask(t->id % pool.num_threads, t);
        }
    }

    // Launch worker threads
    std::vector<std::thread> threads;
    for (int i = 0; i < pool.num_threads; i++) {
        threads.emplace_back(&Scheduler::worker, this, i);
    }

    // Wait for completion
    for (auto &t : threads) {
        t.join();
    }
}