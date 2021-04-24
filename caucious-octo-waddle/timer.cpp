#include "timer.hpp"

Timer::Timer() {
    start = std::chrono::high_resolution_clock::now();
}
Timer::~Timer() {
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << (std::chrono::duration<float>(end - start)).count() * 1000.f << "ms\n";
}