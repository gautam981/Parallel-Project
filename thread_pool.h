#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "dag.h"

/**
 * @brief A work-stealing thread pool.
 * 
 * Each thread maintains its own double-ended queue (std::deque) of tasks.
 * - Local execution uses LIFO (push_back / pop_back) to maximize CPU cache hit rates.
 * - When a thread goes idle, it steals work from other threads using FIFO (pop_front)
 *   to grab the oldest (and likely largest) chunks of the dependency tree.
 */
class ThreadPool {
public:
    int num_threads;
    std::vector<std::deque<std::shared_ptr<Task>>> queues;
    std::vector<std::mutex> queue_mutex;
    
    // Condition variable to put idle threads to sleep, preventing busy-waiting loops (100% CPU lock).
    std::condition_variable cv;
    std::mutex cv_m;

    ThreadPool(int n) : num_threads(n), queues(n), queue_mutex(n) {}

    /**
     * @brief Pushes a ready task into the specified thread's local queue.
     */
    void pushTask(int tid, std::shared_ptr<Task> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex[tid]);
            queues[tid].push_back(task);
        }
        // Notify one sleeping thread (if any) that new work is available
        cv.notify_one();
    }

    /**
     * @brief Pops a task from the local thread's queue.
     * Uses LIFO (Last-In-First-Out) for optimal cache locality.
     */
    std::shared_ptr<Task> popTask(int tid) {
        std::lock_guard<std::mutex> lock(queue_mutex[tid]);
        if (queues[tid].empty()) return nullptr;
        
        // Owner thread takes from the BACK
        auto t = queues[tid].back();
        queues[tid].pop_back();
        return t;
    }

    /**
     * @brief Steals a task from another thread's queue.
     * Uses FIFO (First-In-First-Out) to steal older tasks and minimize interference.
     */
    std::shared_ptr<Task> stealTask(int tid) {
        for (int i = 0; i < num_threads; i++) {
            if (i == tid) continue; // Skip own queue

            std::lock_guard<std::mutex> lock(queue_mutex[i]);
            if (!queues[i].empty()) {
                // Thief thread takes from the FRONT
                auto t = queues[i].front();
                queues[i].pop_front();
                return t;
            }
        }
        return nullptr;
    }
};

#endif