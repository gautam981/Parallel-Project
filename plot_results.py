import csv
import matplotlib.pyplot as plt
import os

def generate_graph():
    if not os.path.exists("results.csv"):
        print("Error: results.csv not found. Please run the benchmark first.")
        return

    threads = []
    time_ms = []
    speedup = []

    # Read the CSV file
    with open("results.csv", "r") as f:
        reader = csv.reader(f)
        next(reader) # skip header
        for row in reader:
            if not row: continue
            threads.append(int(row[0]))
            time_ms.append(float(row[1]))
            speedup.append(float(row[2]))

    plt.figure(figsize=(10, 5))

    # Plot 1: Speedup vs Threads
    plt.subplot(1, 2, 1)
    plt.plot(threads, speedup, marker='o', linestyle='-', color='b', linewidth=2, markersize=8)
    plt.plot(threads, threads, linestyle='--', color='gray', label="Ideal Speedup") # Ideal speedup
    plt.title("Parallel Speedup vs Thread Count", fontsize=12)
    plt.xlabel("Number of Threads", fontsize=10)
    plt.ylabel("Speedup Multiplier (x)", fontsize=10)
    plt.xticks(threads)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()

    # Plot 2: Execution Time vs Threads
    plt.subplot(1, 2, 2)
    plt.plot(threads, time_ms, marker='s', linestyle='-', color='r', linewidth=2, markersize=8)
    plt.title("Execution Time vs Thread Count", fontsize=12)
    plt.xlabel("Number of Threads", fontsize=10)
    plt.ylabel("Execution Time (ms)", fontsize=10)
    plt.xticks(threads)
    plt.grid(True, linestyle='--', alpha=0.6)

    plt.tight_layout()
    plt.savefig("performance_graph.png", dpi=300)
    print("Graph successfully saved as 'performance_graph.png'")

if __name__ == "__main__":
    generate_graph()
