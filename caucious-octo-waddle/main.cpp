#include "vision.hpp"
#include "mandelbrot.hpp"
#include "olc.hpp"
#include <functional>
#include <iostream>

// General purpose oscillator
#define OSC_SINE 0
#define OSC_SQUARE 1
#define OSC_TRIANGLE 2
#define OSC_SAW_ANA 3
#define OSC_SAW_DIG 4
#define OSC_NOISE 5

//#define SMOOTH

double w(double dHertz)
{
	return dHertz * 2.0 * PI;
}

double osc(double dHertz, double dTime, int nType) {
	switch (nType) {
	case OSC_SINE: // Sine wave bewteen -1 and +1
		return sin(w(dHertz) * dTime);

	case OSC_SQUARE: // Square wave between -1 and +1
		return sin(w(dHertz) * dTime) > 0 ? 1.0 : -1.0;

	case OSC_TRIANGLE: // Triangle wave between -1 and +1
		return asin(sin(w(dHertz) * dTime)) * (2.0 / PI);

	case OSC_SAW_ANA: // Saw wave (analogue / warm / slow)
	{
		double dOutput = 0.0;

		for (double n = 1.0; n < 40.0; n++)
			dOutput += (sin(n * w(dHertz) * dTime)) / n;

		return dOutput * (2.0 / PI);
	}

	case OSC_SAW_DIG: // Saw Wave (optimised / harsh / fast)
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));


	case OSC_NOISE: // Pseudorandom noise
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;

	default:
		return 0.0;
	}
}

// Amplitude (Attack, Decay, Sustain, Release) Envelope
struct sEnvelopeADSR {
	double dAttackTime;
	double dDecayTime;
	double dSustainAmplitude;
	double dReleaseTime;
	double dStartAmplitude;
	double dTriggerOffTime;
	double dTriggerOnTime;
	bool bNoteOn;

	sEnvelopeADSR() {
		dAttackTime = 0.10;
		dDecayTime = 0.01;
		dStartAmplitude = 1.0;
		dSustainAmplitude = 0.8;
		dReleaseTime = 0.20;
		bNoteOn = false;
		dTriggerOffTime = 0.0;
		dTriggerOnTime = 0.0;
	}

	// Call when key is pressed
	void NoteOn(double dTimeOn) {
		dTriggerOnTime = dTimeOn;
		bNoteOn = true;
	}

	// Call when key is released
	void NoteOff(double dTimeOff) {
		dTriggerOffTime = dTimeOff;
		bNoteOn = false;
	}

	// Get the correct amplitude at the requested point in time
	double GetAmplitude(double dTime) {
		double dAmplitude = 0.0;
		double dLifeTime = dTime - dTriggerOnTime;

		if (bNoteOn)
		{
			if (dLifeTime <= dAttackTime)
			{
				// In attack Phase - approach max amplitude
				dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;
			}

			if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
			{
				// In decay phase - reduce to sustained amplitude
				dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;
			}

			if (dLifeTime > (dAttackTime + dDecayTime))
			{
				// In sustain phase - dont change until note released
				dAmplitude = dSustainAmplitude;
			}
		}
		else
		{
			// Note has been released, so in release phase
			dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;
		}

		// Amplitude should not be negative
		if (dAmplitude <= 0.0001)
			dAmplitude = 0.0;

		return dAmplitude;
	}
};

// Global synthesizer variables
std::atomic<double> dFrequencyOutput = 0.0;			// dominant output frequency of instrument, i.e. the note
sEnvelopeADSR envelope;							// amplitude modulation of output to give texture, i.e. the timbre
double dOctaveBaseFrequency = 110.0; // A2		// frequency of octave represented by keyboard
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatve

// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
double MakeNoise(double dTime) {
	// Mix together a little sine and square waves
	double dOutput = envelope.GetAmplitude(dTime) *
		(
			+1.0 * osc(dFrequencyOutput * 0.5, dTime, OSC_SINE)
			+ 1.0 * osc(dFrequencyOutput, dTime, OSC_SAW_ANA)
			);

	return dOutput * 0.4; // Master Volume
}

void PlaySounds(MandelData data) {
	// Get all sound hardware
	std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) std::wcout << "Found Output Device: " << d << std::endl;
	std::wcout << "Using Device: " << devices[0] << std::endl;

	// Create sound machine!!
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	// Sit in loop, capturing keyboard state changes and modify
	// synthesizer output accordingly
	int nCurrentKey = -1;
	float k = 1;

	for (size_t i = 0; i < data.topdownPixelView.size(); i++)
	{

#ifdef SMOOTH
		k = ((float)data.topdownPixelView[i].x / data.canvas.cols) * 10;
#else
		k = round(((float)data.topdownPixelView[i].x / data.canvas.cols) * 10);
#endif
		cv::rectangle(data.canvas, cv::Point(
			data.topdownPixelView[i].x - 10, 
			data.topdownPixelView[i].y - 10), 
			cv::Point(
				data.topdownPixelView[i].x + 10, 
				data.topdownPixelView[i].y + 10),
			cv::Scalar(255, 255, 255)
		);

		// If the 
		if (nCurrentKey != k)
		{
			dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
			envelope.NoteOn(sound.GetTime());
			std::printf("Frequency: %uHz, K: %f\n", unsigned(dFrequencyOutput), k);
			nCurrentKey = k;
		}
		Sleep(20);
	}
}

int main() {
    // Process the Mandel Image.
    MandelBrot* mandel = new MandelBrot;
    Vision vision;

    //// Pass lamda function in order for the threads to be able to read class variables. [&]
    std::thread t1([&]() { vision.PlayVision(mandel->GetCanvas().canvas); });
    // std::thread t2([&]() { PlaySounds(mandel->GetCanvas()); });

    //// Start threads.
    t1.join();
    //t2.join();

	return 0;
}