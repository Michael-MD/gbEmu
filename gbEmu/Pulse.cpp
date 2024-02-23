#include "Pulse.hpp"

Pulse::Pulse() : SoundChannel()
{
	PeriodValue = 0;
	PeriodDiv = new Divider<uint16_t>(&PeriodValue, 0x7FF);

}

void Pulse::clock()
{
	PeriodDiv->clock();
}

uint8_t Pulse::GetSample()
{
	// We check if the part of the period reached is 
	// less than the duty cycle value.
	if ((PeriodDiv->nOverflows % 8) <= 4)
	{
		// If within high part of waveform
		return 0x00;
	}
	else
	{
		return 0x0F;
	}
}

Pulse::~Pulse()
{
	delete PeriodDiv;
}