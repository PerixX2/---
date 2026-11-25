#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

const int ITERATIONS = 1900000;

int counterMutexVal = 0;
std::mutex counterMutex;

void incrementMutex() {
    for (int i = 0; i < ITERATIONS; ++i) {
        std::lock_guard<std::mutex> lock(counterMutex);
        ++counterMutexVal;
    }
}

std::atomic<int> counterAtomicVal{0};

void incrementAtomic() {
    for (int i = 0; i < ITERATIONS; ++i) {
        ++counterAtomicVal;
    }
}

int main() {
    const int expected = 2 * ITERATIONS;

    std::cout << "=== std::mutex (5 runs, N = 1900000 per thread) ===\n";
    double totalMutexMs = 0.0;
    for (int run = 1; run <= 5; ++run) {
        counterMutexVal = 0;
        auto start = std::chrono::steady_clock::now();
        std::thread t1(incrementMutex);
        std::thread t2(incrementMutex);
        t1.join();
        t2.join();
        auto end = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        totalMutexMs += ms;
        std::cout << "Run " << run << ": Expected = " << expected
                  << ", Actual = " << counterMutexVal
                  << ", Time = " << ms << " ms\n";
    }

    std::cout << "\n=== std::atomic (5 runs, N = 1900000 per thread) ===\n";
    double totalAtomicMs = 0.0;
    for (int run = 1; run <= 5; ++run) {
        counterAtomicVal = 0;
        auto start = std::chrono::steady_clock::now();
        std::thread t1(incrementAtomic);
        std::thread t2(incrementAtomic);
        t1.join();
        t2.join();
        auto end = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        totalAtomicMs += ms;
        std::cout << "Run " << run << ": Expected = " << expected
                  << ", Actual = " << counterAtomicVal.load()
                  << ", Time = " << ms << " ms\n";
    }

    std::cout << "\nAverage time (std::mutex):  " << (totalMutexMs / 5.0) << " ms\n";
    std::cout << "Average time (std::atomic): " << (totalAtomicMs / 5.0) << " ms\n";
    return 0;
}
