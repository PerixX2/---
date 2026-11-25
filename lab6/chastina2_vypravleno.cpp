#include <iostream>
#include <thread>
#include <mutex>
#include <barrier>

std::mutex mutexA;
std::mutex mutexB;
std::barrier syncPoint1(2);
std::barrier syncPoint2(2);

void task1_scoped() {
    syncPoint1.arrive_and_wait();
    std::scoped_lock lock(mutexA, mutexB);
    std::cout << "[scoped_lock] Thread 1: both mutexes locked\n";
}

void task2_scoped() {
    syncPoint1.arrive_and_wait();
    std::scoped_lock lock(mutexB, mutexA);
    std::cout << "[scoped_lock] Thread 2: both mutexes locked\n";
}

void task1_ordered() {
    syncPoint2.arrive_and_wait();
    std::lock_guard<std::mutex> lockA(mutexA);
    std::lock_guard<std::mutex> lockB(mutexB);
    std::cout << "[ordered] Thread 1: both mutexes locked\n";
}

void task2_ordered() {
    syncPoint2.arrive_and_wait();
    std::lock_guard<std::mutex> lockA(mutexA);
    std::lock_guard<std::mutex> lockB(mutexB);
    std::cout << "[ordered] Thread 2: both mutexes locked\n";
}

int main() {
    std::cout << "=== Method 1: std::scoped_lock ===\n";
    {
        std::thread t1(task1_scoped);
        std::thread t2(task2_scoped);
        t1.join();
        t2.join();
    }

    std::cout << "\n=== Method 2: Unified lock order (mutexA -> mutexB) ===\n";
    {
        std::thread t1(task1_ordered);
        std::thread t2(task2_ordered);
        t1.join();
        t2.join();
    }

    std::cout << "\nProgram finished without deadlock.\n";
    return 0;
}
