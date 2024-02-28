#include "SoundChannel.hpp"
#include "GB.hpp"

SoundChannel::SoundChannel()
{
	PeriodValue = 0xC1;
	PeriodDiv = new Divider<uint16_t>(&PeriodValue, 0x7FF);

	// Length Counter
	LenCount = 0;

	// Turn everything off by default
	Mute = true;
	SweepOn = false;
	EnvelopeOn = false;
	LenCounterOn = false;

	Volume = 7;

	DACon = false;
}

void SoundChannel::trigger()
{
	// Turn channel on
	Mute = false;
	
	// Set length counter
	// TODO: Set to 256 for wave channel
	if (NRx1->InitLenTimer == 0)
	{
		NRx1->InitLenTimer = 64;
	}

	// TODO: Reset frequency timer with period

	// Reload period value
	PeriodValue = ((NRx4->Period << 8) | *NRx3) & 0x7FF;

	// TODO: Volume envelope timer reloaded with period

	// Channel volume reloaded from NRx2
	Volume = NRx2->InitVol;

	// TODO: Noise LFSR are all set to 1
	// TODO: Wave channel's position is set to 0 but sample buffer is NOT refilled.

	// Turn all internal units off. Although this isn't
	// what actually happens, it will produce similar 
	// behaviour based on the way we have done things here.
	SweepOn = false;
	EnvelopeOn = false;
	LenCounterOn = false;

	// If DAC is off the channel will immediately be turned
	// back off.
	if (!DACon)
	{
		Mute = true;
	}
}

SoundChannel::~SoundChannel()
{
	delete PeriodDiv;
}

void SoundChannel::connectGB(GB* gb)
{
	this->gb = gb;
}