#pragma once
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <portaudio.h>
#include <complex> // for complex numbers
#include <iostream>

#include <cstddef>
#include <cstdint>

int constexpr TABLE_SIZE = 400;
int constexpr SAMPLE_RATE = 44100;
int constexpr FRAMES_PER_BUFFER = 64;
int constexpr BLOCKING_MILLISECONDS = 200;
int constexpr M_PI = 3.14159265;

double constexpr RE_MIN = -2.4;
double constexpr RE_MAX = 1.5;
double constexpr IM_MIN = -1.3;
double constexpr IM_MAX = 1.3;

//size_t constexpr LY = 60 / 2.54 * 300;
// Resolution
size_t constexpr LY = 1000;

double constexpr DIA = (IM_MAX - IM_MIN) / LY;
size_t constexpr LX = (RE_MAX - RE_MIN) / DIA;

// Accuracy
int constexpr MAX_ITER = 50;
double constexpr RADIUS = 5;

size_t constexpr VECLEN = 4;
size_t constexpr BLOCKS_X = LX / VECLEN;

uint8_t transform(int x, int max, int to) {
    return std::log(x) * to / std::log(max);
}

size_t idx(size_t const bx, size_t const y) {
    return y * BLOCKS_X + bx;
}

double get_c_re(size_t const bx, size_t const x) {
    return RE_MIN + (bx * VECLEN + x) * DIA;
}

double get_c_im(size_t const y) {
    return IM_MIN + y * DIA;
}

struct soa {
    alignas(VECLEN * sizeof(double)) double z[2][VECLEN];
    // alignas(VECLEN * sizeof(int)) int z[VECLEN] = {0};
};

struct PortData {
    float pitch;
    unsigned long n;
};

struct PixelCoordinates {
    int x;
    int y;
};
struct MandelData{
    cv::Mat canvas;
    std::vector<PixelCoordinates> topdownPixelView;
};

struct Timer{
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    Timer(){
        start = std::chrono::high_resolution_clock::now();
    }
    ~Timer(){
        end = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken: " << (std::chrono::duration<float>(end - start)).count() * 1000.f << "ms\n";
    }
};