#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "dag.h"
#include "thread_pool.h"
#include <atomic>
#include <thread>   // ✅ ADD THIS

class Scheduler {
public:
    DAG &dag;
    ThreadPool pool;
    std::atomic<int> remaining;

    Scheduler(DAG &d, int threads);

    void run();

private:
    void worker(int tid);   // ✅ FIXED
    void executeTask(std::shared_ptr<Task> task);
};

#endif