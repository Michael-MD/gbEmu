#include "Noise.hpp"
#include "GB.hpp"

Noise::Noise(uint8_t ChannelNum) : SoundChannel(ChannelNum)
{
	LFSR = 0xFFFF;
}

void Noise::clock()
{
	// Increments divider which controls 
	// period duration of wave.

	// If divider = 0 then it is treated as 0.5
	uint8_t ModOp = 4 + NR43->ClockShift;
	if (NR43->ClockDiv == 0)
	{
		ModOp -= 1;
	}

	if (gb->nClockCycles % ModOp == 0)
	{
		// Shift LFSR
		if ((++ClockEntrances % (NR43->ClockDiv == 0 ? 1 : NR43->ClockDiv)) == 0)
		{
			ClockEntrances = 0;

			// XOR 2 least significant and place.
			bool XORRes = LFSR.Bit0 ^ LFSR.Bit1;

			// Place result in appropriate slots
			if (NR43->LFSRWidth) // Short mode
			{
				LFSR.Bit7 = XORRes;
			}

			LFSR.Bit15 = XORRes;

			// Shift the entire shift register to the right
			LFSR.reg >>= 1;
		}
	}

	// ================= Length Counter ================= 
	if (!LenCounterOn && NR44->LenEnable)
	{
		LenCount = NR41->InitLenTimer;
		LenCounterOn = true;
	}

	if (!NR44->LenEnable)
	{
		LenCounterOn = false;
	}

	if (gb->nClockCycles % (1 << 14) == 0 && NR44->LenEnable)	// Called at 256Hz
	{
		if (LenCount++ == 64)
		{
			LenCount = NR41->InitLenTimer;
			Mute = true;
			gb->apu.NR52->bCH4 = 0;
		}
	}

	// ================= Envelope ================= 
	// Disables envelope if sweep pace is zero
	if (NR42->SweepPace == 0)
	{
		EnvelopeOn = false;
	}

	// Switch envelope on
	if (NR42->SweepPace != 0 && !EnvelopeOn)
	{
		EnvelopeOn = true;
		EnvelopeEntrances = 0;
		Volume = NR42->InitVol;
	}

	// Disables channel if bits 3-7 are all zero.
	if (NR42->InitVol == 0 && NR42->EnvDir == 0)
	{
		Mute = true;
		gb->apu.NR52->bCH1 = 0;
	}

	// TODO: Fix EnvelopeEntrances stuff
	if (EnvelopeOn && gb->nClockCycles % (1 << 16) == 0 && (++EnvelopeEntrances % NR42->SweepPace == 0))	// Called at 64Hz
	{
		// Based on the pace we increment the volume
		if (NR42->EnvDir == 0)	// Decrease volume
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

uint8_t Noise::GetSample()
{
	return 0;

	if (Mute)
	{
		return 0;
	}

	if (LFSR.Bit0 == 1)
	{
		return Volume;
	}

	return 0;
}

void Noise::trigger()
{

}

Noise::~Noise()
{

}