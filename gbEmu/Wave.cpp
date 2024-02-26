#include "Wave.hpp"
#include "GB.hpp"

Wave::Wave() : SoundChannel()
{

}

void Wave::clock()
{
	// Increments divider which controls 
	// period duration of wave.
	if (gb->nClockCycles % 2 == 0)	// Clocked at 4.19Mhz/2
	{
		PeriodDiv->clock();
	}

	// Checks if DAC is enabled or disabled 
	// which enables or mutes the channel
	if (NR30->bDAC == 0)
	{
		Mute = true;
		gb->apu.NR52->bCH3 = 0;
	}
	else
	{
		Mute = false;
		gb->apu.NR52->bCH3 = 1;
	}

	// ================= Length Counter ================= 
	if (!LenCounterOn && NR34->LenEnable)
	{
		LenCount = *NR31;
		LenCounterOn = true;
	}

	if (!NR34->LenEnable)
	{
		LenCounterOn = false;
	}

	if (gb->nClockCycles % (1 << 14) == 0 && NR34->LenEnable)	// Called at 256Hz
	{
		if (LenCount++ == 64)
		{
			LenCount = *NR31;
			Mute = true;
			gb->apu.NR52->bCH3 = 0;
		}
	}
}

uint8_t Wave::GetSample()
{
	// Don't output anything if 
	//		- channel muted,
	//		- the volume is 0
	//		- the wave has been played
	if (Mute || NR32->OutLvl == 0 || PatternInd >= 32)
	{
		return 0;
	}

	// Read byte from pattern RAM.
	uint8_t ByteSample = PatternRAM[PatternInd >> 1];

	// Now since pattern RAM contains two samples in the 
	// low and high nybbles of the byte we need
	// to determine which byte we need.
	uint8_t NybbleSample = ByteSample >> (PatternInd % 2) * 4;
	NybbleSample &= 0x0F;

	// The sample volume needs to be modulated, here is this 
	// done using only 4 levels which are determined by NR32
	// which determines by how much to shift the nybble.
	NybbleSample >>= (NR32->OutLvl - 1);

	// NybbleSample will be within 0x0 - 0xF
	return NybbleSample;

	// Finally increment the index of the pattern RAM for
	// next time.
	PatternInd++;
}

Wave::~Wave()
{

}