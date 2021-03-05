#include "main.h"

using namespace cv;

static int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    PortData* data = (PortData*)userData;
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

static void DisplayImageWindow(Mat img) {
    // ConstantLY refresh the board on a different thread, there is probabLY a better way of doing this.
    while (true) {
        imshow("Image", img);
        waitKey(1);
    }
}

static MandelData preprocessing() {
    Timer timer;
    MandelData image;
    std::vector<uint8_t> escape_iter(LX * LY, MAX_ITER);

    #pragma omp parallel for collapse(2)
    for (size_t y = 0; y < LY; ++y) {
        for (size_t bx = 0; bx < BLOCKS_X; ++bx) {
            alignas(VECLEN * sizeof(bool)) bool finished[VECLEN] = { false };
            alignas(VECLEN * sizeof(bool)) bool written[VECLEN] = { false };

            alignas(VECLEN * sizeof(double)) double z_re[VECLEN] = { 0.0 };
            alignas(VECLEN * sizeof(double)) double z_im[VECLEN] = { 0.0 };
            alignas(VECLEN * sizeof(double)) double new_z_re[VECLEN];
            alignas(VECLEN * sizeof(double)) double new_z_im[VECLEN];

            alignas(VECLEN * sizeof(double)) double c_re[VECLEN] = { 0.0 };
            for (size_t v = 0; v < VECLEN; ++v) {
                c_re[v] = get_c_re(bx, v);
            }
            double c_im = get_c_im(y);

            for (size_t iter = 0; iter < MAX_ITER; ++iter) {

                #pragma omp simd aligned(z_re, z_im, new_z_re, new_z_im, finished)
                for (size_t v = 0; v < VECLEN; ++v) {
                    new_z_re[v] =
                        z_re[v] * z_re[v] - z_im[v] * z_im[v] + c_re[v];
                    new_z_im[v] = 2 * z_re[v] * z_im[v] + c_im;

                    z_re[v] = new_z_re[v];
                    z_im[v] = new_z_im[v];

                    auto const abs = z_re[v] * z_re[v] + z_im[v] * z_im[v];

                    if (!finished[v] && (abs > RADIUS)) {
                        finished[v] = true;
                        escape_iter[idx(bx, y) * VECLEN + v] = iter;
                    }
                }

                bool all_finished = true;
                for (size_t v = 0; v < VECLEN; ++v) {
                    all_finished &= finished[v];
                }

                if (all_finished) {
                    break;
                }
            }
        }
    }

    image.canvas = *new cv::Mat(LY, LX, CV_8UC1, Scalar(0, 0, 0));
    for (size_t bx = 0; bx < BLOCKS_X; bx++)
    {
        bool top_pixel_found = false;
        for (size_t y = 0; y < LY; y++){
        
            for (size_t v = 0; v < VECLEN; v++)
            {
                uint8_t transformed = transform(escape_iter[idx(bx, y) * VECLEN + v], MAX_ITER, 255);
                int x = bx * VECLEN + v;
                if (transformed == 255) {
                    if (!top_pixel_found){
                        image.topdownPixelView.push_back(*new PixelCoordinates{ x,static_cast<int>(y) });
                        top_pixel_found = true;
                    }
                }
                image.canvas.at<uint8_t>(y, x) = transformed;
            }
        }
    }
    return image;
}

static void PlayAudio(MandelData img) {
    PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;

    PortData data;
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

    float splitter = 10.f / img.canvas.rows;


    for (auto& e : img.topdownPixelView)
    {
        rectangle(img.canvas, Point(e.x - 10, e.y - 10), Point(e.x+10, e.y+10), Scalar(255, 255, 255));
        data.pitch = splitter * e.y;
        PlaySound(stream);
    }
 
    CloseStream(stream);
}

int main() {
    printf("Generating mandelbrot, this can take a while...\n");
    MandelData img = preprocessing();
    std::thread ImageDisplayThread(DisplayImageWindow, img.canvas);
    std::thread PlayAudioThread(PlayAudio, img);
    ImageDisplayThread.join();
    PlayAudioThread.join();
    
	return 0;
}