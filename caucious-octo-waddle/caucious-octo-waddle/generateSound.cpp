#include <portaudio.h>
#include <iostream>

#define TABLE_SIZE (400)
#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (64)

#ifndef M_PI
#define M_PI (3.14159265)
#endif


typedef struct paTestData
{ 
    unsigned long n;
    char message[20];
};

static int patestCallback(const void* inputBuffer, void* outputBuffer,unsigned long framesPerBuffer,const PaStreamCallbackTimeInfo* timeInfo,PaStreamCallbackFlags statusFlags,void* userData){
    paTestData* data = (paTestData*)userData;
    float* out = (float*)outputBuffer;

    (void)timeInfo;
    (void)statusFlags;
    (void)inputBuffer;
    for (unsigned long i = 0; i < framesPerBuffer; i++, data->n++) {
        float v = sin(1800 * 2 * M_PI * ((float)data->n / (float)TABLE_SIZE));
        printf("Data value: %lu\n",data->n);
        *out++ = v;
        *out++ = v;

        // Optimisations.
        //if (v >= TABLE_SIZE) v -= TABLE_SIZE;
        //if (v >= TABLE_SIZE) v -= TABLE_SIZE;
    }
    return paContinue;
}

static void StreamFinished(void* userData)
{
    paTestData* data = (paTestData*)userData;
    printf("Stream Completed: %s\n", data->message);
}

static int initPortAudio() {
    PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;
    paTestData data;

    int i;

    printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

    /* initialise sinusoidal wavetable */
    data.n = 0;

    err = Pa_Initialize();

    if (err != paNoError)
    {
        printf("Error in Initialize:-", err);
        goto error;
    }

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output     device */

    if (outputParameters.device == paNoDevice)
    {
        fprintf(stderr, "Error: No default output device.\n");
        goto error;
    }

    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream
    (
        &stream,
        NULL, /* no input */
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        patestCallback,
        &data
    );

    if (err != paNoError)
        goto error;

    sprintf_s(data.message, 11, "No Message");
    err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);

    if (err != paNoError)
        goto error;

    err = Pa_StartStream(stream);

    if (err != paNoError)
        goto error;

    printf("Play for %d seconds.\n", 1000);
    Pa_Sleep(1000);

    err = Pa_StopStream(stream);

    if (err != paNoError)
        goto error;

    err = Pa_CloseStream(stream);

    if (err != paNoError)
        goto error;

    Pa_Terminate();
    printf("Test finished.\n");

    return err;

error:
    Pa_Terminate();

    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));

    return err;
}