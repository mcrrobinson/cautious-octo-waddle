#pragma once
#include <cstddef>
#include <cstdint>
#include <complex>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "mandeldata.h"

class MandelBrot
{
public:
    MandelBrot();

    MandelData GetCanvas();

    // Create canvas object.
    MandelData canvas;

    // RE minimums.
    double const RE_MIN = -2.4;
    double const RE_MAX = 1.5;
    double const IM_MIN = -1.3;
    double const IM_MAX = 1.3;

    size_t const LY = 1000;
    double const DIA = (IM_MAX - IM_MIN) / LY;
    size_t const LX = (RE_MAX - RE_MIN) / DIA;

    // Accuracy
    int const MAX_ITER = 50;
    double const RADIUS = 5;

    static size_t const VECLEN = 4;
    size_t const BLOCKS_X = LX / VECLEN;

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
        //alignas(VECLEN * sizeof(int)) int z[VECLEN] = {0};
    };
};