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
	NR52->reg = 0x70;

	NR51 = reinterpret_cast<NR51Register*>(gb->RAM + 0xFF25);
	NR50 = reinterpret_cast<NR50Register*>(gb->RAM + 0xFF24);

	// Channel 1 - Pulse 1
	pulse1.NR10 = reinterpret_cast<Pulse::NR10Register*>(gb->RAM + 0xFF10);
	pulse1.NR11 = reinterpret_cast<Pulse::NR11Register*>(gb->RAM + 0xFF11);
	pulse1.NR12 = reinterpret_cast<Pulse::NR12Register*>(gb->RAM + 0xFF12);
	pulse1.NR13 = gb->RAM + 0xFF13;
	pulse1.NR14 = reinterpret_cast<Pulse::NR14Register*>(gb->RAM + 0xFF14);

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
		
		// TODO: Volume control
		// Maps 0 -> 3 in master volume register to 
		// range 1 - 6000.
		LeftChannel *= 500;
		RightChannel *= 500;


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


uint8_t APU::read(uint16_t addr)
{
	// addr >= 0xFF10 && addr <= 0xFF3F

	if (addr == 0xFF10)		// NR11: Channel 1 length timer & duty cycle
	{
		// Initial length timer cannot be read
		return ~(11 << 6) ^ (pulse1.NR11->Duty << 6);
	}
	else if (addr == 0xFF14)	// NR14: Channel 1 period high & control
	{
		// Only length enable bit can be read.
		// Everything else is 1.
		return ~(1 << 6) ^ (pulse1.NR14->LenEnable << 6);
	}

	return gb->RAM[addr];
}

void APU::write(uint16_t addr, uint8_t data)
{
	// addr >= 0xFF10 && addr <= 0xFF3F

	if (addr == 0xFF12)	// NR12: Channel 1 volume & envelope
	{
		// If initial volume is changed then we want to restart the sweep unit
		// so that it can latch this new value
		if (data >> 4 != pulse1.NR12->InitVol)
		{
			pulse1.SweepOn = false;
		}

		*pulse1.NR12 = data;
		
	}
	else if (addr == 0xFF13)	// NR13 - Pulse channel 1 Period value low byte
	{
		*pulse1.NR13 = data;
		pulse1.PeriodValue = ((pulse1.NR14->Period << 8) | *pulse1.NR13) & 0x7FF;
	}
	else if (addr == 0xFF14)	// NR14 - Pulse channel 1 various control bits
	{
		*pulse1.NR14 = data;
		// Check if channel 1 should be turned on
		if (pulse1.NR14->Trigger == 1)
		{
			pulse1.Mute = false;
			pulse1.SweepOn = false;
			pulse1.EnvelopeOn = false;
			pulse1.LenCounterOn = false;
			NR52->bCH1 = 1;
		}
		pulse1.PeriodValue = ((pulse1.NR14->Period << 8) | *pulse1.NR13) & 0x7FF;
	}
	else if (addr == 0xFF26)	// Audio master control
	{
		NR52->bAPU = data >> 7;
	}
	else
	{
		gb->RAM[addr] = data;
	}
}