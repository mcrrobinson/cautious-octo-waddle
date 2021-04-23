#pragma once
#include <portaudio.h>
#include "mandeldata.h"

class Audio
{
public:
	int const TABLE_SIZE = 400;
	int const SAMPLE_RATE = 44100;
	int const FRAMES_PER_BUFFER = 64;
	int const BLOCKING_MILLISECONDS = 200;
	double const M_PI = 3.14159265;

	struct PortData {
		float pitch;
		unsigned long n;
		double pi = 3.14159265;
		int table_size = 400;
	};

	PaStreamParameters outputParameters;
	PaStream* stream;
	PaError err;

	PortData data;

	void PlaySound(PaStream* stream);
	void CloseStream(PaStream* stream);
	void ErrorExit(const char* errorMessage, int err);
	void PlayAudio(MandelData img);

	int GetTableSize();
	int GetSampleRate();
	int GetFramesBuffer();
	double GetPi();
};