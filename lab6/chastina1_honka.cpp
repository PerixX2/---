#include <iostream>
#include <thread>

int counter = 0;
const int ITERATIONS = 1900000;

void increment() {
    for (int i = 0; i < ITERATIONS; ++i) {
        ++counter;
    }
}

int main() {
    const int expected = 2 * ITERATIONS;
    int mismatches = 0;

    for (int run = 1; run <= 10; ++run) {
        counter = 0;
        std::thread t1(increment);
        std::thread t2(increment);
        t1.join();
        t2.join();

        bool ok = (counter == expected);
        if (!ok) ++mismatches;
        std::cout << "Run " << run << ": Expected = " << expected
                  << ", Actual = " << counter
                  << (ok ? " [OK]" : " [DATA RACE]") << '\n';
    }

    std::cout << "Mismatches out of 10 runs: " << mismatches << '\n';
    return 0;
}
