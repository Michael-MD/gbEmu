#include "APU.hpp"
#include "GB.hpp"

APU::APU()
{

}

APU::~APU()
{

}

void APU::connectGB(GB* gb)
{
	this->gb = gb;

	// Reference APU Registers to Main Registers Block
	NR52 = reinterpret_cast<NR52Register*>(gb->RAM + 0xFF26);
	NR51 = reinterpret_cast<NR51Register*>(gb->RAM + 0xFF25);
	NR50 = reinterpret_cast<NR50Register*>(gb->RAM + 0xFF24);

	// Channel 1 - Pulse 1
	pulse1.NR10 = reinterpret_cast<Pulse::NR10Register*>(gb->RAM + 0xFF10);
	pulse1.NR11 = reinterpret_cast<Pulse::NR11Register*>(gb->RAM + 0xFF11);
	pulse1.NR12 = reinterpret_cast<Pulse::NR12Register*>(gb->RAM + 0xFF12);
	pulse1.NR13 = gb->RAM + 0xFF13;
	pulse1.NR14 = reinterpret_cast<Pulse::NR14Register*>(gb->RAM + 0xFF14);

	*pulse1.NR13 = 0xC1;

	pulse1.connectGB(gb);
}

void APU::AudioSample(void* userdata, Uint8* stream, int len)
{

	APU* apu = static_cast<APU*>(userdata);

	Sint16* buffer = reinterpret_cast<Sint16*>(stream);
	int nSamples = len / sizeof(Sint16);

	// Place holder for analog value output
	// by DAC.
	Sint16 AnalogVal = 0;

	// Mixer
	Sint32 RightChannel = 0, LeftChannel = 0;

	for (int j = 0; j < nSamples; j += 2) 
	{
		// Run emulation until next sample
		for (int k = 0; k < 1000 * 4.19 / 44.1; k++)
		{
			apu->gb->clock();
		}

		LeftChannel = 0;
		RightChannel = 0;

		// Get sample
		AnalogVal = apu->pulse1.GetSample();
		
		// Mixer
		if (apu->NR51->CH1R)
		{
			RightChannel += AnalogVal;
		}
		else if (apu->NR51->CH1L)
		{
			LeftChannel += AnalogVal;
		}
		
		// Volume control
		// Maps 0 -> 3 in master volume register to 
		// range 1 - 6000.
		LeftChannel = LeftChannel * 1000;
		RightChannel = RightChannel * 1000;


		buffer[j] = LeftChannel;
		buffer[j + 1] = RightChannel;

	}
}

void APU::clock()
{
	if (!NR52->bAPU)
	{
		return;
	}

	// Increment Dividers
	pulse1.clock();

	// The APU has an internal divider
	// which ticks on the falling edge of
	// the fourth bit. So if clocked in increments
	// at 4.19MHz then the counter increments at
	// 4.19MHz / 2^3 ~= 512Hz.
	
}
