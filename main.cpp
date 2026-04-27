#include <iostream>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <string>
#include "dag.h"
#include "scheduler.h"

using namespace std;
using namespace chrono;

/**
 * @brief Simulates workload for a sequential baseline.
 * We run an empty loop to mimic computational effort for each task.
 */
void runSequential(DAG &dag) {
    for (size_t i = 0; i < dag.tasks.size(); i++) {
        volatile long long sum = 0;
        for (int j = 0; j < 1000000; j++) { // 1M loop iterations simulating work
            sum += j;
        }
    }
}

/**
 * @brief Generates a complex DAG structure (a complete binary tree).
 * A binary tree ensures that tasks branch out, providing true parallel 
 * opportunities unlike a simple sequential chain of tasks.
 */
void generateTreeDAG(DAG &dag, int n) {
    // Node i depends on (or must finish before) 2*i+1 and 2*i+2
    for (int i = 0; i < n; i++) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < n) dag.addDependency(i, left);
        if (right < n) dag.addDependency(i, right);
    }
}

int main(int argc, char* argv[]) {
    // Default to 1000 tasks, but allow user to override via command line
    int n = 1000;
    if (argc > 1) {
        try {
            n = std::stoi(argv[1]);
            if (n <= 0) throw std::invalid_argument("Negative or zero tasks");
        } catch (...) {
            cerr << "Error: Please provide a valid positive integer for the number of tasks.\n";
            cerr << "Usage: " << argv[0] << " [num_tasks]\n";
            return 1;
        }
    }

    cout << "=========================================================\n";
    cout << "🚀 ThreadX - Parallel DAG Task Scheduler Benchmark 🚀\n";
    cout << "=========================================================\n";
    cout << "Tasks: " << n << " (Binary Tree Dependency Structure)\n\n";

    DAG dag_seq(n);
    generateTreeDAG(dag_seq, n);

    // Sequential Run
    cout << "Running Sequential Baseline...\n";
    auto start_seq = high_resolution_clock::now();
    runSequential(dag_seq);
    auto end_seq = high_resolution_clock::now();
    auto seq_time = duration_cast<microseconds>(end_seq - start_seq).count();

    cout << "✅ Sequential Time: " << seq_time / 1000.0 << " ms\n\n";

    ofstream file("results.csv");
    file << "Threads,Time(ms),Speedup\n";

    cout << "+---------+-----------------+-------------+\n";
    cout << "| Threads |    Time (ms)    |   Speedup   |\n";
    cout << "+---------+-----------------+-------------+\n";

    cout << fixed << setprecision(2);

    // Test across typical multi-core hardware thread counts
    vector<int> thread_counts = {1, 2, 4, 8, 16};
    
    for (int t : thread_counts) {
        // We create a fresh DAG for each parallel run because task execution 
        // dynamically decrements atomic dependency counters to 0. 
        // Reusing the same DAG would skip execution.
        DAG dag_par(n);
        generateTreeDAG(dag_par, n);
        
        auto start = high_resolution_clock::now();
        
        // Initialize the scheduler and start thread pool execution
        Scheduler scheduler(dag_par, t);
        scheduler.run();
        
        auto end = high_resolution_clock::now();
        
        auto par_time = duration_cast<microseconds>(end - start).count();
        double par_time_ms = par_time / 1000.0;
        double speedup = (double)seq_time / par_time;

        cout << "| " << setw(7) << t << " | " << setw(12) << par_time_ms << " ms | " << setw(10) << speedup << "x |\n";
        file << t << "," << par_time_ms << "," << speedup << "\n";
    }

    cout << "+---------+-----------------+-------------+\n\n";
    cout << "Results saved to 'results.csv'. Run 'python plot_results.py' to visualize.\n";
    cout << "=========================================================\n";
    
    file.close();
    return 0;
}
