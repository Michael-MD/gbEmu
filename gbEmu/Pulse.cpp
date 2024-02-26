#include "Pulse.hpp"
#include "GB.hpp"

Pulse::Pulse() : SoundChannel()
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

void Pulse::clock()
{
	// Increments divider which controls 
	// period duration of wave.
	if (gb->nClockCycles % 4 == 0)
	{
		PeriodDiv->clock();
	}

	// Check if DAC is off, according to 
	// PanDocs it is off if and only if
	// NRx2 & 0xF8 != 0.
	if ((NRx2->reg & 0xF8) == 0)
	{
		Mute = true;
		gb->apu.NR52->bCH1 = 0;
	}

	// ================= Length Counter ================= 
	if (!LenCounterOn && NRx4->LenEnable)
	{
		LenCount = NRx1->InitLenTimer;
		LenCounterOn = true;
	}

	if (!NRx4->LenEnable)
	{
		LenCounterOn = false;
	}

	if (gb->nClockCycles % (1 << 14) == 0 && NRx4->LenEnable)	// Called at 256Hz
	{
		if (LenCount++ == 64)
		{
			LenCount = NRx1->InitLenTimer;
			Mute = true;
			gb->apu.NR52->bCH1 = 0;
		}
	}

	// ================= Sweep Unit ================= 

	// Check if sweep unit just turned on
	if (!SweepOn && NRx0->Pace != 0)
	{
		SweepOn = true;
		CurrentPace = NRx0->Pace;
		SweepEntrances = 0;
	}

	// If Pace is set to 0 then immediately stop 
	// the sweep.
	if (NRx0->Pace == 0)
	{
		SweepOn = false;
	}

	// If sweep is on then increment period value at pace
	if (SweepOn && (gb->nClockCycles % (1 << 15)) == 0 && (++SweepEntrances % CurrentPace == 0))	// Called at 128Hz / NRx0->Pace
	{
		uint16_t DeltaP = PeriodValue >> NRx0->Step;

		if(NRx0->Direction == 0)	// Addition
		{
			PeriodValue += DeltaP;

		}
		else	// Subtraction
		{
			if(PeriodValue >= DeltaP)
			{
				PeriodValue -= DeltaP;
			}
		}

		// Disable the channel immediately if overflows
		if (PeriodValue > 0x7FF)
		{
			Mute = true;
			gb->apu.NR52->bCH1 = 0;
		}

		// Update NRx3 and NRx4
		*NRx3 = PeriodValue & 0xFF;
		NRx4->Period = PeriodValue >> 8;
	}

	// ================= Envelope ================= 
	// Disables envelope if sweep pace is zero
	if (NRx2->SweepPace == 0)
	{
		EnvelopeOn = false;
	}

	// Switch envelope on
	if (NRx2->SweepPace != 0 && !EnvelopeOn)
	{
		EnvelopeOn = true;
		EnvelopeEntrances = 0;
		Volume = NRx2->InitVol;
	}

	// Disables channel if bits 3-7 are all zero.
	if (NRx2->InitVol == 0 && NRx2->EnvDir == 0)
	{
		Mute = true;
		gb->apu.NR52->bCH1 = 0;
	}

	if (EnvelopeOn && gb->nClockCycles % (1 << 16) == 0 && (++EnvelopeEntrances % NRx2->SweepPace == 0))	// Called at 64Hz
	{
		// Based on the pace we increment the volume
		if (NRx2->EnvDir == 0)	// Decrease volume
		{
			// Avoid underflow
			if (Volume > 0)
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

uint8_t Pulse::GetSample()
{
	if (Mute)
	{
		return 0;
	}

	// We check if the part of the period reached is 
	// less than the duty cycle value.
	if ((PeriodDiv->nOverflows % 8) <= (NRx1->Duty << 1))
	{
		// If within high part of waveform
		return Volume;
	}
	else
	{
		return 0;
	}
}

Pulse::~Pulse()
{
	delete PeriodDiv;
}