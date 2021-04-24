#pragma once
#include <iostream>
#include "olc.hpp"

// General purpose oscillator
#define OSC_SINE 0
#define OSC_SQUARE 1
#define OSC_TRIANGLE 2
#define OSC_SAW_ANA 3
#define OSC_SAW_DIG 4
#define OSC_NOISE 5

class Audio
{
public:
	Audio();
	~Audio();
	double w(double dHertz);
	double osc(double dHertz, double dTime, int nType = OSC_SINE);
	double MakeNoise(double dTime);
	void PlaySounds();
};

Audio::Audio()
{
	PlaySounds();
}

Audio::~Audio()
{
}
