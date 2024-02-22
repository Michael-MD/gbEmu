#include "APU.hpp"
#include "GB.hpp"

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
}

void APU::AudioSample(void* userdata, Uint8* stream, int len)
{
	static uint64_t i = 0;

	Sint16* buffer = reinterpret_cast<Sint16*>(stream);
	int nSamples = len / sizeof(Sint16);

	for (int j = 0; j < nSamples; j += 2) {
		buffer[j] = 28000 * sin(2 * M_PI * 440 * (i++) / 44100);
		buffer[j+1] = buffer[j];
	}
}

void APU::clock()
{
	if (!NR52->bAPU)
	{
		return;
	}


}
