#include "mandelbrot.hpp"
#include "timer.hpp"

MandelBrot::MandelBrot()
{
    printf("Generating mandelbrot, this can take a while...\n");
    Timer timer;
    std::vector<uint8_t> escape_iter(LX * this->LY, this->MAX_ITER);

    // Adding openmp adds threading for the following loops bettering performance.
    // Specifies how many loops in a nested loop should be collapsed into a 
    // iteration space and divided. Increases performance but results in duplication
    // code leading to a larger binary.
#pragma omp parallel for collapse(2)
    for (size_t y = 0; y < this->LY; ++y) {
        for (size_t bx = 0; bx < this->BLOCKS_X; ++bx) {
            alignas(this->VECLEN * sizeof(bool)) bool finished[this->VECLEN] = { false };
            alignas(this->VECLEN * sizeof(bool)) bool written[this->VECLEN] = { false };

            alignas(this->VECLEN * sizeof(double)) double z_re[this->VECLEN] = { 0.0 };
            alignas(this->VECLEN * sizeof(double)) double z_im[this->VECLEN] = { 0.0 };
            alignas(this->VECLEN * sizeof(double)) double new_z_re[this->VECLEN];
            alignas(this->VECLEN * sizeof(double)) double new_z_im[this->VECLEN];

            alignas(this->VECLEN * sizeof(double)) double c_re[this->VECLEN] = { 0.0 };
            for (size_t v = 0; v < this->VECLEN; ++v) {
                c_re[v] = this->get_c_re(bx, v);
            }
            double c_im = this->get_c_im(y);

            for (size_t iter = 0; iter < this->MAX_ITER; ++iter) {

                // Modern CPU's support the AVX instruction set and can do operations with four floating 
                // point numbers (double precision) at the same time. Since each pixel gets this approximation
                // this could be taken advantage of.
#pragma omp simd aligned(z_re, z_im, new_z_re, new_z_im, finished)
                for (size_t v = 0; v < this->VECLEN; ++v) {
                    new_z_re[v] =
                        z_re[v] * z_re[v] - z_im[v] * z_im[v] + c_re[v];
                    new_z_im[v] = 2 * z_re[v] * z_im[v] + c_im;

                    z_re[v] = new_z_re[v];
                    z_im[v] = new_z_im[v];

                    auto const abs = z_re[v] * z_re[v] + z_im[v] * z_im[v];

                    // For this we take an array escape_iter which is linearized.
                    if (!finished[v] && (abs > this->RADIUS)) {
                        finished[v] = true;
                        escape_iter[this->idx(bx, y) * this->VECLEN + v] = iter;
                    }
                }

                bool all_finished = true;
                for (size_t v = 0; v < this->VECLEN; ++v) {
                    all_finished &= finished[v];
                }

                if (all_finished) {
                    break;
                }
            }
        }
    }

    this->canvas.canvas = *new cv::Mat(this->LY, this->LX, CV_8UC1, cv::Scalar(0, 0, 0));
    for (size_t bx = 0; bx < this->BLOCKS_X; bx++)
    {
        bool top_pixel_found = false;
        for (size_t y = 0; y < this->LY; y++) {

            for (size_t v = 0; v < this->VECLEN; v++)
            {
                uint8_t transformed = this->transform(escape_iter[this->idx(bx, y) * this->VECLEN + v], this->MAX_ITER, 255);
                int x = bx * this->VECLEN + v;
                if (transformed == 255) {
                    if (!top_pixel_found) {
                        this->canvas.topdownPixelView.push_back(*new PixelCoordinates{ x, static_cast<int>(y) });
                        top_pixel_found = true;
                    }
                }
                this->canvas.canvas.at<uint8_t>(y, x) = transformed;
            }
        }
    }
}

MandelData MandelBrot::GetCanvas()
{
    return this->canvas;
}
