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
	// Increments divider which controls 
	// period duration of wave.
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
			gb->apu.NR52->bCH1 = 0;
		}
	}

	//// ================= Sweep Unit ================= 

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

	// If sweep is on then increment period value at pace
	if (SweepOn && (gb->nClockCycles % (32 * NR10->Pace)) == 0)	// Called at 128Hz * NR10->Pace
	{
		if(NR10->Direction == 0)	// Addition
		{
			PeriodValue += PeriodValue >> NR10->Step;

		}
		else	// Subtraction
		{
			if(PeriodValue != 0)
			{
				PeriodValue -= PeriodValue >> NR10->Step;
			}
		}

		// Disable the channel immediately if overflows
		if (PeriodValue > 0x7FF)
		{
			Mute = true;
			gb->apu.NR52->bCH1 = 0;
		}

		// Update NR13 and NR14
		*NR13 = PeriodValue & 0xFF;
		NR14->Period = PeriodValue >> 8;
	}

	// ================= Envelope ================= 
	// Disables envelope if sweep pace is zero
	if (NR12->SweepPace == 0)
	{
		EnvelopeOn = false;
	}

	// Switch envelope on
	if (NR12->SweepPace != 0 && !EnvelopeOn)
	{
		EnvelopeOn = true;
		Volume = NR12->InitVol;
	}

	// Disables channel if bits 3-7 are all zero.
	if (NR12->InitVol == 0 && NR12->EnvDir == 0)
	{
		Mute = true;
		gb->apu.NR52->bCH1 = 0;
	}

	if (EnvelopeOn && gb->nClockCycles % (64 * NR12->SweepPace) == 0)	// Called at 64Hz * NR12->SweepPace
	{
		if (NR12->EnvDir == 0)	// Decrease volume
		{
			// Avoid underflow
			if (Volume >= 0)
			{
				Volume -= 1;
			}
		}
		else	// Increase volume
		{
			// Avoid overflow
			if (Volume < 16)
			{
				Volume += 1;
			}
		}
	}
}

int8_t Pulse::GetSample()
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
		return Volume;
	}
	else
	{
		return -Volume;
	}
}

Pulse::~Pulse()
{
	delete PeriodDiv;
}