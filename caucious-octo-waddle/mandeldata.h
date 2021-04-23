#pragma once
#include <vector>
#include <opencv2/core/mat.hpp>
struct PixelCoordinates {
    int x;
    int y;
};

struct MandelData {
    cv::Mat canvas;
    std::vector<PixelCoordinates> topdownPixelView;
};