#pragma once
#include <chrono>
#include <iostream>

struct Timer {
    std::chrono::time_point<std::chrono::steady_clock> start, end;

    Timer();
    ~Timer();
};