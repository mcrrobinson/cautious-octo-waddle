#include "vision.hpp"
#include "mandelbrot.hpp"
#include "olc.hpp"
#include <functional>
#include <iostream>

// 12th root of 2. Used in the calculation of getting
// a piano frequency from index of keys. Rather CPU
// intensive so using a constant.
#define ROOT_CALC_CONST 1.059463094

// Delay in Ms between X axis transition.
#define DELAY 5

// Used to change the frequency. Run in background too.
// Best make it atomic, we don't want no race conditions do we!
std::atomic<double>dFrequencyOutput = 0.0;

// 88 keys on a standard piano. 49 is middle C.
double PianoFrequency(int key = 49)
{
    return (std::pow(ROOT_CALC_CONST, (key-49)) * 440);
}

double MakeNoise(double dTime)
{
    double dOutput = 1 * sin(dFrequencyOutput * 2 * PI * dTime);

    // Returns square wave, sounds like shit but doesn't have the knocking.
    if (dOutput > 0) return 0.1;
    else return -0.1;
}

void PlaySounds(MandelData data)
{
    int k;
    std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
    olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);
    sound.SetUserFunction(MakeNoise);

    for (size_t i = 0; i < data.topdownPixelView.size(); i++){

        // This gross function is actually pretty simple...
        // It divides the current Y axis by total columns to get a value inbetween 0
        // and 1. It's converted to a float so the compiler knows thats what's
        // meant to be output. Then taken away from one so that the number is inverted
        // because to us higher up represents the larger number, yeah CV does it the
        // other way. Finally times it by 88 as there are 88 keys on a standard keyboard
        // and round it to a solid int.
        k = round(
            (1- static_cast<float>(data.topdownPixelView[i].y) / data.canvas.cols) * 88
        );
        cv::rectangle(data.canvas, cv::Point(
            data.topdownPixelView[i].x - 10,
            data.topdownPixelView[i].y - 10),
            cv::Point(
                data.topdownPixelView[i].x + 10,
                data.topdownPixelView[i].y + 10),
            cv::Scalar(255, 255, 255)
        );
        dFrequencyOutput = PianoFrequency(k);
        printf("Key: %d, Freq: %f, X: %d, Y: %d\n", k, (double)dFrequencyOutput, data.topdownPixelView[i].x, data.topdownPixelView[i].y);
        Sleep(DELAY);
    }
    exit(1);
}

int main() {
    // Process the Mandel Image.
    MandelBrot* mandel = new MandelBrot;
    Vision vision;

    // Pass lamda function in order for the threads to be able to read class variables. [&]
    std::thread t1([&]() { vision.PlayVision(mandel->GetCanvas().canvas); });
    std::thread t2([&]() { PlaySounds(mandel->GetCanvas()); });

    //// Start threads.
    t1.join();
    t2.join();

	return 0;
}