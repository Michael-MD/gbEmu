#include "SoundChannel.hpp"
#include "GB.hpp"

SoundChannel::SoundChannel()
{
	PeriodValue = 0;
	PeriodDiv = new Divider<uint16_t>(&PeriodValue, 0x7FF);

	// Length Counter
	LenCount = 0;

	// Turn everything off by default
	Mute = true;
	SweepOn = false;
	EnvelopeOn = false;
	LenCounterOn = false;

	Volume = 7;
}

SoundChannel::~SoundChannel()
{
	delete PeriodDiv;
}

void SoundChannel::connectGB(GB* gb)
{
	this->gb = gb;
}