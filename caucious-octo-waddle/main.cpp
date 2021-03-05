#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <portaudio.h>
#include <complex> // for complex numbers
#include <iostream>

#define TABLE_SIZE (400)
#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (64)
#define BLOCKING_MILLISECONDS (200)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

// This is the resolution
#define WIDTH (1200)
#define HEIGHT (1200)

using namespace cv;

typedef struct paTestData
{
    float pitch;
    unsigned long n;
};

typedef struct imageStuff
{
    Mat image;
    int columnValue[WIDTH]{};
};

struct Timer
{
    std::chrono::time_point<std::chrono::steady_clock> start, end;

    Timer()
    {
        start = std::chrono::high_resolution_clock::now();
    }
    ~Timer()
    {
        end = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken: " << (std::chrono::duration<float>(end - start)).count() * 1000.f << "ms\n";
    }
};

static int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    paTestData* data = (paTestData*)userData;
    float* out = (float*)outputBuffer;

    (void)timeInfo;
    (void)statusFlags;
    (void)inputBuffer;
    for (unsigned long i = 0; i < framesPerBuffer; i++, data->n++) {
        float v = sin(data->pitch * 2 * M_PI * ((float)data->n / (float)TABLE_SIZE));
        *out++ = v;
        *out++ = v;

        // Optimisations.
        if (v >= TABLE_SIZE) v -= TABLE_SIZE;
        if (v >= TABLE_SIZE) v -= TABLE_SIZE;
    }
    return paContinue;
}

static void ErrorExit(const char* errorMessage, int err) {
    Pa_Terminate();

    fprintf(stderr, errorMessage);
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
}

static void StreamFinished(void* userData)
{
    //printf("Stream Completed.\n");
}

static void PlaySound(PaStream* stream) {
    int err;

    err = Pa_StartStream(stream);
    if (err != paNoError) ErrorExit("Failed to start stream.",err);
    Pa_Sleep(BLOCKING_MILLISECONDS);

    err = Pa_StopStream(stream);
    if (err != paNoError) ErrorExit("Failed to stop stream.",err);
}

static void CloseStream(PaStream* stream) {
    Pa_CloseStream(stream);
    Pa_Terminate();
}

static void PortAudio(float pitch) {
    PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;

    paTestData data;
    data.n = 0;
    data.pitch = pitch;
}

static void DisplayImageWindow(Mat img) {
    // Constantly refresh the board on a different thread, there is probably a better way of doing this.
    while (true) {
        imshow("Image", img);
        waitKey(1);
    }
}

static imageStuff preprocessing() {
    Timer timer;
    imageStuff image;
    image.image = *new Mat(WIDTH, HEIGHT, CV_8UC3, Scalar(0, 0, 0));
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            std::complex<float> point((float)x / WIDTH - 1.5, (float)y / HEIGHT - 0.5);
            std::complex<float> z(0, 0);
            int nb_iter = 0;
            while (abs(z) < 2 && nb_iter <= 20) {
                z = z * z + point;
                nb_iter++;
            }
            if (nb_iter < 20) image.image.at<Vec3b>(Point(x, y)) = Vec3b(0, 0, 0);
            else {
                if (image.columnValue[x] == NULL) image.columnValue[x] = y;
                image.image.at<Vec3b>(Point(x, y)) = Vec3b(255, 255, 255);
            }
        }
    }
    return image;
}

static void restShit(imageStuff img) {
    PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;

    paTestData data;
    data.n = 0;

    err = Pa_Initialize();
    if (err != paNoError) ErrorExit("Failed to initalise PortAudio", err);

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) ErrorExit("Failed to get default output device", err);

    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream
    (
        &stream,
        NULL,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        patestCallback,
        &data
    );
    if (err != paNoError) ErrorExit("Failed to open output stream.", err);

    err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
    if (err != paNoError) ErrorExit("Stream finished callback failed | %i", err);

    float splitter = 10.f / img.image.rows;
    for (size_t i = 0; i < WIDTH; i++)
    {
        rectangle(img.image, Point(i - 5, img.columnValue[i] - 5), Point(i, img.columnValue[i]), Scalar(255, 255, 255));
        data.pitch = splitter * img.columnValue[i];
        PlaySound(stream);
    }
    CloseStream(stream);
}

int main() {
    printf("Generating mandelbrot, this can take a while...\n");
    imageStuff img = preprocessing();
    std::thread t1(DisplayImageWindow, img.image);
    std::thread t2(restShit, img);
    t1.join();
    t2.join();
    
	return 0;
}