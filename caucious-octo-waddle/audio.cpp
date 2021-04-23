#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <portaudio.h>
#include <complex> // for complex numbers
#include <iostream>

#include <cstddef>
#include <cstdint>

#include "audio.h"

int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    Audio::PortData* data = (Audio::PortData*)userData;
    float* out = (float*)outputBuffer;

    (void)timeInfo;
    (void)statusFlags;
    (void)inputBuffer;
    for (unsigned long i = 0; i < framesPerBuffer; i++, data->n++) {
        float v = sin(data->pitch * 2 * data->pi * ((float)data->n / (float)data->table_size));
        *out++ = v;
        *out++ = v;

        // Optimisations.
        if (v >= data->table_size) { v -= data->table_size; }
        if (v >= data->table_size) { v -= data->table_size; }
    }
    return paContinue;
}

void StreamFinished(void* userData)
{
    printf("Stream Completed.\n");
}

void Audio::PlaySound(PaStream* stream)
{
    int err;

    err = Pa_StartStream(stream);
    if (err != paNoError) ErrorExit("Failed to start stream.", err);
    Pa_Sleep(GetTableSize());

    err = Pa_StopStream(stream);
    if (err != paNoError) ErrorExit("Failed to stop stream.", err);
}

void Audio::PlayAudio(MandelData img) {
    this->data.n = 0;
    this->data.table_size = this->TABLE_SIZE;
    this->data.pi = this->M_PI;

    this->err = Pa_Initialize();
    if (this->err != paNoError) ErrorExit("Failed to initalise PortAudio", this->err);

    this->outputParameters.device = Pa_GetDefaultOutputDevice();
    if (this->outputParameters.device == paNoDevice) ErrorExit("Failed to get default output device", this->err);

    this->outputParameters.channelCount = 2;
    this->outputParameters.sampleFormat = paFloat32;
    this->outputParameters.suggestedLatency = Pa_GetDeviceInfo(this->outputParameters.device)->defaultLowOutputLatency;
    this->outputParameters.hostApiSpecificStreamInfo = NULL;

    this->err = Pa_OpenStream
    (
        &this->stream,
        NULL,
        &this->outputParameters,
        GetSampleRate(),
        GetFramesBuffer(),
        paClipOff,
        patestCallback,
        &this->data
    );
    if (this->err != paNoError) ErrorExit("Failed to open output stream.", this->err);

    this->err = Pa_SetStreamFinishedCallback(this->stream, &StreamFinished);
    if (this->err != paNoError) ErrorExit("Stream finished callback failed | %i", this->err);

    float splitter = 10.f / img.canvas.rows;


    for (auto& e : img.topdownPixelView)
    {
        rectangle(img.canvas, cv::Point(e.x - 10, e.y - 10), cv::Point(e.x + 10, e.y + 10), cv::Scalar(255, 255, 255));
        this->data.pitch = splitter * e.y;
        PlaySound(this->stream);
    }
}

void Audio::ErrorExit(const char* errorMessage, int err) {
    Pa_Terminate();
    fprintf(stderr, errorMessage);
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
}

void Audio::CloseStream(PaStream* stream) {
    Pa_CloseStream(stream);
    Pa_Terminate();
}

int Audio::GetTableSize() {
    return this->TABLE_SIZE;
}

int Audio::GetSampleRate() {
    return this->SAMPLE_RATE;
}

int Audio::GetFramesBuffer() {
    return this->FRAMES_PER_BUFFER;
}

double Audio::GetPi() {
    return this->M_PI;
}