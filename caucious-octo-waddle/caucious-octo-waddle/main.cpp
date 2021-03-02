#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <portaudio.h>
#include <iostream>

#define TABLE_SIZE (400)
#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (64)
#define BLOCKING_MILLISECONDS (200)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

using namespace cv;

typedef struct paTestData
{
    float pitch;
    unsigned long n;
};

static int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    paTestData* data = (paTestData*)userData;
    float* out = (float*)outputBuffer;

    (void)timeInfo;
    (void)statusFlags;
    (void)inputBuffer;
    for (unsigned long i = 0; i < framesPerBuffer; i++, data->n++) {
        float v = sin(data->pitch * 2 * M_PI * ((float)data->n / (float)TABLE_SIZE));
        //printf("Data value: %lu\n", data->n);
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
    printf("Stream Completed.\n");
}

static void PlaySound(PaStream* stream) {
    int err;

    err = Pa_StartStream(stream);
    if (err != paNoError) ErrorExit("Failed to start stream.",err);

    printf("Blocking for for %d milliseconds.\n", BLOCKING_MILLISECONDS);
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

static Mat preprocessing(std::string path, int threshold, int mandel_left_flip_value, int mandel_right_flip_value) {
    Mat image1, image1_binary_map, mandel_left, mandel_right, concat_binary_map;

    image1 = imread(path, 0);
    image1_binary_map = image1 > threshold;

    mandel_left = image1_binary_map(
        Range(0, round(image1_binary_map.rows / 2)),  // Defines the upper half of the image.
        Range(0, image1_binary_map.cols)
    );

    mandel_right = image1_binary_map(
        Range(round(image1_binary_map.rows / 2), image1_binary_map.rows), // Defines the lower side of the image.
        Range(0, image1_binary_map.cols)
    );

    // If a flip value has been declared flip the image.
    if (mandel_left_flip_value != NULL) flip(mandel_left, mandel_left, mandel_left_flip_value);
    if (mandel_right_flip_value != NULL) flip(mandel_right, mandel_right, mandel_right_flip_value);
    hconcat(mandel_left, mandel_right, concat_binary_map);

    return concat_binary_map;
}

static void restShit(Mat img) {
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

    float splitter = 10.f / img.rows;
    for (int i = 0; i < img.cols; i++) {
        for (int j = 0; j < img.rows; j++) {
            if (img.at<uchar>(j, i) == 255) {
                try
                {
                    // Draw a rectangle and play corresponding pitch sound.
                    rectangle(img, Point(i - 5, j - 5), Point(i,j), Scalar(255));
                    data.pitch = splitter * j;
                    PlaySound(stream);
                }
                catch (const std::exception&)
                {
                    printf("%i | %i\n", i, j);
                }

                break;
            }
        }
    }

    CloseStream(stream);
}

int main() {
    const int threshold = 128;
    const std::string path = "Resources/mandel-small.jpg";
    const int mandel_left_flip_value = NULL;
    const int mandel_right_flip_value = -1;

    

    Mat img = preprocessing(
        path,
        threshold,
        mandel_left_flip_value,
        mandel_right_flip_value
    );

    std::thread t1(DisplayImageWindow, img);
    std::thread t2(restShit, img);
    t1.join();
    t2.join();
    
	return 0;
}