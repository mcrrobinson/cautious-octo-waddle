#include "audio.hpp"
#include "vision.hpp"
#include "mandelbrot.hpp"
#include <functional>
#include <iostream>


int main() {
    // Process the Mandel Image.
    //MandelBrot* mandel = new MandelBrot;
    //Vision vision;
    Audio audio;

    //// Pass lamda function in order for the threads to be able to read class variables. [&]
    //std::thread t1([&]() { vision.PlayVision(mandel->GetCanvas().canvas); });
    std::thread t1([&]() { audio.PlaySounds(); });

    //// Start threads.
    //t1.join();

	return 0;
}