#include "Pulse.hpp"
#include "GB.hpp"

Pulse::Pulse() : SoundChannel()
{
	PeriodValue = 0;
	PeriodDiv = new Divider<uint16_t>(&PeriodValue, 0x7FF);

	// Length Counter
	LenCount = 0;

}

void Pulse::clock()
{
	if (gb->nClockCycles % 4 == 0)
	{
		PeriodDiv->clock();
	}

	// ================= Length Counter ================= 
	if (!LenCounterOn && NR14->LenEnable)
	{
		LenCount = NR11->InitLenTimer;
		LenCounterOn = true;
	}

	if (gb->nClockCycles % 16 == 0 && NR14->LenEnable)	// Called at 256Hz
	{
		if (LenCount++ == 64)
		{
			LenCount = NR11->InitLenTimer;
			Mute = true;
		}
	}

	// ================= Sweep Unit ================= 

	// Check if sweep unit just turned on
	if (SweepOn == 0 && NR10->Pace != 0)
	{
		SweepOn = true;
		CurrentPace = NR10->Pace;
	}

	// If Pace is set to 0 then immediately stop 
	// the sweep.
	if (CurrentPace == 0)
	{
		SweepOn = false;
	}

	// If sweep is on then increment period value
	if (SweepOn && gb->nClockCycles % (32 * NR10->Pace) == 0)	// Called at 128Hz * NR10->Pace
	{
		if(NR10->Direction == 0)	// Addition
		{
			PeriodValue += PeriodValue >> NR10->Step;

		}
		else	// Subtraction
		{
			PeriodValue -= PeriodValue >> NR10->Step;
		}

		// Disable the channel immediately if value under or overflows
		if (PeriodValue > 0x7FF)
		{
			Mute = false;
		}

		// Update NR13 and NR14
		*NR13 = PeriodValue & 0xFF;
		NR14->Period = PeriodValue >> 8;
	}
}

int16_t Pulse::GetSample()
{
	if (Mute)
	{
		return 0;
	}

	// We check if the part of the period reached is 
	// less than the duty cycle value.
	if ((PeriodDiv->nOverflows % 8) <= (NR11->Duty << 1))
	{
		// If within high part of waveform
		return 1;
	}
	else
	{
		return -1;
	}
}

Pulse::~Pulse()
{
	delete PeriodDiv;
}