#include <iostream>
#include <vector>
#include <future>
#include <chrono>
#include <cmath>
#include <iomanip>

double dotProductRange(const std::vector<double>& a, const std::vector<double>& b, size_t start, size_t end) {
    double sum = 0.0;
    for (size_t i = start; i < end; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

double dotProductSequential(const std::vector<double>& a, const std::vector<double>& b) {
    return dotProductRange(a, b, 0, a.size());
}

double dotProductParallel(const std::vector<double>& a, const std::vector<double>& b, size_t numTasks) {
    size_t n = a.size();
    if (numTasks == 0) numTasks = 1;
    if (numTasks > n) numTasks = n;

    std::vector<std::future<double>> futures;
    futures.reserve(numTasks);

    size_t chunkSize = n / numTasks;
    size_t remainder = n % numTasks;
    size_t currentStart = 0;

    for (size_t t = 0; t < numTasks; ++t) {
        size_t currentEnd = currentStart + chunkSize + (t < remainder ? 1 : 0);
        futures.push_back(std::async(std::launch::async, dotProductRange, std::cref(a), std::cref(b), currentStart, currentEnd));
        currentStart = currentEnd;
    }

    double total = 0.0;
    for (auto& f : futures) {
        total += f.get();
    }
    return total;
}

int main() {
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads\n\n";

    std::vector<size_t> sizes = {1000000, 10000000, 50000000};
    std::vector<size_t> taskCounts = {2, 4, 8};

    std::cout << std::fixed << std::setprecision(3);

    for (size_t n : sizes) {
        std::vector<double> a(n);
        std::vector<double> b(n);
        for (size_t i = 0; i < n; ++i) {
            a[i] = static_cast<double>((i % 100) + 1) * 0.01;
            b[i] = static_cast<double>(((i + 7) % 100) + 1) * 0.02;
        }

        auto seqStart = std::chrono::steady_clock::now();
        double seqResult = dotProductSequential(a, b);
        auto seqEnd = std::chrono::steady_clock::now();
        double seqMs = std::chrono::duration<double, std::milli>(seqEnd - seqStart).count();

        std::cout << "=== Vector size N = " << n << " ===\n";
        std::cout << "Sequential: Result = " << seqResult << ", Time = " << seqMs << " ms\n";

        for (size_t tasks : taskCounts) {
            auto parStart = std::chrono::steady_clock::now();
            double parResult = dotProductParallel(a, b, tasks);
            auto parEnd = std::chrono::steady_clock::now();
            double parMs = std::chrono::duration<double, std::milli>(parEnd - parStart).count();

            double speedup = seqMs / parMs;
            double efficiency = speedup / static_cast<double>(tasks);
            double diff = std::abs(seqResult - parResult);

            std::cout << "Tasks = " << tasks
                      << " | Parallel Time = " << parMs << " ms"
                      << " | Speedup S = " << speedup
                      << " | Efficiency E = " << efficiency
                      << " | Diff = " << diff << "\n";
        }
        std::cout << '\n';
    }

    return 0;
}
