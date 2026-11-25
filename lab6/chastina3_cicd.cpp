#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <chrono>
#include <vector>

struct BuildJob {
    int id;
    std::string project;
    int durationUnits;
    bool willSucceed;
};

std::queue<BuildJob> jobQueue;
std::mutex queueMutex;
std::mutex outputMutex;
std::condition_variable condition;
bool finished = false;
size_t maxObservedQueueSize = 0;

const int JOB_COUNT = 12;

void producer(bool useNotifyOne) {
    const std::string projects[] = {
        "AuthService",
        "PaymentGateway",
        "CoreEngine",
        "AnalyticsWorker"
    };

    for (int i = 1; i <= JOB_COUNT; ++i) {
        BuildJob job{
            i,
            projects[(i - 1) % 4],
            (i % 3) + 1,
            (i % 5 != 0)
        };

        size_t currentSize = 0;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            jobQueue.push(job);
            currentSize = jobQueue.size();
            if (currentSize > maxObservedQueueSize) {
                maxObservedQueueSize = currentSize;
            }
        }

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << "[Producer] Queued CI/CD job #" << job.id
                      << " | Project: " << job.project
                      << " | Complexity: " << job.durationUnits
                      << " | Queue size: " << currentSize << '\n';
        }

        if (useNotifyOne) {
            condition.notify_one();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        finished = true;
    }
    condition.notify_all();

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "[Producer] All " << JOB_COUNT << " CI/CD jobs dispatched.\n";
    }
}

void consumer(int runnerId, int delayMultiplierMs) {
    while (true) {
        BuildJob job;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, []() {
                return !jobQueue.empty() || finished;
            });

            if (jobQueue.empty() && finished) {
                break;
            }

            job = jobQueue.front();
            jobQueue.pop();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(job.durationUnits * delayMultiplierMs));

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << "[Runner " << runnerId << "] Built job #" << job.id
                      << " (" << job.project << ") -> Status: "
                      << (job.willSucceed ? "SUCCESS" : "FAILED (exit code 1)") << '\n';
        }
    }

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "[Runner " << runnerId << "] Stopped.\n";
    }
}

void runSimulation(int runnerCount, int delayMultiplierMs, bool useNotifyOne) {
    while (!jobQueue.empty()) jobQueue.pop();
    finished = false;
    maxObservedQueueSize = 0;

    std::cout << "=== CI/CD Pipeline (" << runnerCount
              << " runner(s), delay = " << delayMultiplierMs
              << " ms, notify_one = " << (useNotifyOne ? "ON" : "OFF") << ") ===\n";

    auto start = std::chrono::steady_clock::now();

    std::thread prodThread(producer, useNotifyOne);
    std::vector<std::thread> runners;
    for (int i = 1; i <= runnerCount; ++i) {
        runners.emplace_back(consumer, i, delayMultiplierMs);
    }

    prodThread.join();
    for (auto& r : runners) {
        r.join();
    }

    auto end = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "Total time: " << ms << " ms | Max queue size: " << maxObservedQueueSize << "\n\n";
}

int main() {
    runSimulation(2, 150, true);
    runSimulation(1, 150, true);
    runSimulation(4, 150, true);
    runSimulation(2, 350, true);
    runSimulation(2, 150, false);
    return 0;
}
