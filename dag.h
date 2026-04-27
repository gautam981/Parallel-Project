#ifndef DAG_H
#define DAG_H

#include <vector>
#include <memory>
#include <atomic>

using namespace std;

struct Task {
    int id;
    vector<int> dependents;
    atomic<int> dependency_count;

    Task(int id) : id(id), dependency_count(0) {}
};

class DAG {
public:
    vector<shared_ptr<Task>> tasks;

    DAG(int n) {
        for (int i = 0; i < n; i++) {
            tasks.push_back(make_shared<Task>(i));
        }
    }

    void addDependency(int u, int v) {
        tasks[u]->dependents.push_back(v);
        tasks[v]->dependency_count++;
    }
};

#endif