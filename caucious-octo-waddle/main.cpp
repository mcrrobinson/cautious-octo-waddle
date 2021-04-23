#include "audio.h"
#include "vision.h"
#include "mandelbrot.h"
#include <functional>
#include <iostream>

int main() {
    // Process the Mandel Image.
    MandelBrot* mandel = new MandelBrot;
    Audio audio;
    Vision vision;

    // Pass lamda function in order for the threads to be able to read class variables. [&]
    std::thread t1([&]() { vision.PlayVision(mandel->GetCanvas().canvas); });
    std::thread t2([&]() { audio.PlayAudio(mandel->GetCanvas()); });

    // Start threads.
    t1.join();
    t2.join();

	return 0;
}