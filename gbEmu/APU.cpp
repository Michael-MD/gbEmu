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
	pulse1.NRx0 = reinterpret_cast<Pulse::NRx0Register*>(gb->RAM + 0xFF10);
	pulse1.NRx1 = reinterpret_cast<Pulse::NRx1Register*>(gb->RAM + 0xFF11);
	pulse1.NRx2 = reinterpret_cast<Pulse::NRx2Register*>(gb->RAM + 0xFF12);
	pulse1.NRx3 = gb->RAM + 0xFF13;
	pulse1.NRx4 = reinterpret_cast<Pulse::NRx4Register*>(gb->RAM + 0xFF14);

	// Channel 2 - Pulse 2
	// Pulse channel 2 doesn't have a sweep to we simply map it
	// to a random fixed memory location to avoid memory leaks.
	pulse2.NRx0 = reinterpret_cast<Pulse::NRx0Register*>(&NRx20);
	pulse2.NRx1 = reinterpret_cast<Pulse::NRx1Register*>(gb->RAM + 0xFF16);
	pulse2.NRx2 = reinterpret_cast<Pulse::NRx2Register*>(gb->RAM + 0xFF17);
	pulse2.NRx3 = gb->RAM + 0xFF18;
	pulse2.NRx4 = reinterpret_cast<Pulse::NRx4Register*>(gb->RAM + 0xFF19);

	// Channel 3 - Wave Output
	wave.NR30 = reinterpret_cast<Wave::NR30Register*>(gb->RAM + 0xFF1A);
	wave.NR31 = gb->RAM + 0xFF1B;
	wave.NR32 = reinterpret_cast<Wave::NR32Register*>(gb->RAM + 0xFF1C);
	wave.NR33 = gb->RAM + 0xFF1D;
	wave.NR34 = reinterpret_cast<Wave::NR34Register*>(gb->RAM + 0xFF1E);
	wave.PatternRAM = gb->RAM + 0xFF30;

	// Channel 4 - Noise
	noise.NR41 = reinterpret_cast<Noise::NR41Register*>(gb->RAM + 0xFF20);
	noise.NR42 = reinterpret_cast<Noise::NR42Register*>(gb->RAM + 0xFF21);
	noise.NR43 = reinterpret_cast<Noise::NR43Register*>(gb->RAM + 0xFF22);
	noise.NR44 = reinterpret_cast<Noise::NR44Register*>(gb->RAM + 0xFF23);

	Channels[0] = &pulse1;
	Channels[1] = &pulse2;
	Channels[2] = &wave;
	Channels[3] = &noise;

	// Pass gb pointer to channels
	for (size_t i = 0; i < nChannels; i++)
	{
		Channels[i]->connectGB(gb);
	}

}

void APU::AudioSample(void* userdata, Uint8* stream, int len)
{

	APU* apu = static_cast<APU*>(userdata);

	Sint16* buffer = reinterpret_cast<Sint16*>(stream);
	int nSamples = len / sizeof(Sint16);

	// Placeholder for analog value output
	// by DAC.
	uint8_t AnalogVal = 0;

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

		// Loop over all channels
		for (size_t i = 0; i < nChannels; i++)
		{
			// Get sample
			AnalogVal = apu->Channels[i]->GetSample();

			// =========== Mixer =========== 
			// Channel right sterio output
			if ((apu->NR51->reg >> i) & 1)
			{
				RightChannel += AnalogVal;
			}

			// Channel left sterio output
			if ((apu->NR51->reg >> (i + 4)) & 1)
			{
				LeftChannel += AnalogVal;
			}
		}

		// The master volume register NR50 contains
		// a scale value for the left and right channels.
		// Note we have added 1 since 0 should not mute the
		// channel.
		LeftChannel *= 50 * (apu->NR50->VolL + 1);
		RightChannel *= 50 * (apu->NR50->VolR + 1);


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

	// Pass clock signal to each channel
	for (size_t i = 0; i < nChannels; i++)
	{
		Channels[i]->clock();
	}

}


uint8_t APU::read(uint16_t addr)
{
	// addr >= 0xFF10 && addr <= 0xFF3F

	if (addr == 0xFF10)		// NRx1: Channel 1 length timer & duty cycle
	{
		// Initial length timer cannot be read
		return ~(11 << 6) ^ (pulse1.NRx1->Duty << 6);
	}
	else if (addr == 0xFF14)	// NRx4: Channel 1 period high & control
	{
		// Only length enable bit can be read.
		// Everything else is 1.
		return ~(1 << 6) ^ (pulse1.NRx4->LenEnable << 6);
	}
	else if (addr == 0xFF1D)	// NR33: Channel 3 period low [write-only]
	{
		return 0xFF;
	}
	else if (addr == 0xFF1E)	// NR34: Channel 3 period high & control
	{
		// Only length enable bit can be read.
		// Everything else is 1.
		return ~(1 << 6) ^ (pulse1.NRx4->LenEnable << 6);
	}

	return gb->RAM[addr];
}

void APU::write(uint16_t addr, uint8_t data)
{
	// addr >= 0xFF10 && addr <= 0xFF3F

	// APU registers cannot be written to 
	// while it is off except NR52 to turn
	// it on.
	if (NR52->bAPU == 0 && addr != 0xFF26)
	{
		return;
	}

	if (addr == 0xFF12)	// NR12: Channel 1 volume & envelope
	{
		// If initial volume is changed then we want to restart the sweep unit
		// so that it can latch this new value
		if (data >> 4 != pulse1.NRx2->InitVol)
		{
			pulse1.EnvelopeOn = false;
		}

		*pulse1.NRx2 = data;
	}
	else if (addr == 0xFF13)	// NR13 - Pulse channel 1 Period value low byte
	{
		*pulse1.NRx3 = data;
		pulse1.PeriodValue = ((pulse1.NRx4->Period << 8) | *pulse1.NRx3) & 0x7FF;
	}
	else if (addr == 0xFF14)	// NR14 - Pulse channel 1 various control bits
	{
		*pulse1.NRx4 = data;
		// Check if channel 1 should be turned on
		if (pulse1.NRx4->Trigger == 1)
		{
			pulse1.Mute = false;
			pulse1.SweepOn = false;
			pulse1.EnvelopeOn = false;
			pulse1.LenCounterOn = false;
			NR52->bCH1 = 1;
		}
		pulse1.PeriodValue = ((pulse1.NRx4->Period << 8) | *pulse1.NRx3) & 0x7FF;
	}
	else if (addr == 0xFF17)	// NR22: Channel 2 volume & envelope
	{
		// If initial volume is changed then we want to restart the sweep unit
		// so that it can latch this new value
		if (data >> 4 != pulse2.NRx2->InitVol)
		{
			pulse2.EnvelopeOn = false;
		}

		*pulse2.NRx2 = data;
	}
	else if (addr == 0xFF18)	// NR23 - Pulse channel 2 Period value low byte
	{
		*pulse2.NRx3 = data;
		pulse2.PeriodValue = ((pulse2.NRx4->Period << 8) | *pulse2.NRx3) & 0x7FF;
	}
	else if (addr == 0xFF19)	// NR24 - Pulse channel 2 various control bits
	{
		*pulse2.NRx4 = data;
		// Check if channel 1 should be turned on
		if (pulse2.NRx4->Trigger == 1)
		{
			pulse2.Mute = false;
			pulse2.SweepOn = false;
			pulse2.EnvelopeOn = false;
			pulse2.LenCounterOn = false;
			NR52->bCH2 = 1;
		}
		pulse2.PeriodValue = ((pulse2.NRx4->Period << 8) | *pulse2.NRx3) & 0x7FF;
	}
	else if (addr == 0xFF21)	// NR42: Channel 4 volume & envelope
	{
		// If initial volume is changed then we want to restart the sweep unit
		// so that it can latch this new value
		if (data >> 4 != noise.NR42->InitVol)
		{
			noise.EnvelopeOn = false;
		}

		*noise.NR42 = data;
	}
	else if (addr == 0xFF23)	// NR44: Channel 4 control
	{
		*noise.NR44 = data;
		// Check if channel 1 should be turned on
		if (pulse2.NRx4->Trigger == 1)
		{
			noise.Mute = false;
			noise.SweepOn = false;
			noise.EnvelopeOn = false;
			noise.LenCounterOn = false;
			NR52->bCH4 = 1;
		}
	}
	else if (addr == 0xFF26)	// Audio master control
	{
		NR52->bAPU = data >> 7;
	}
	else if (addr == 0xFF1D)	// NR33: Channel 3 period low [write-only]
	{
		*wave.NR33 = data;
		wave.PeriodValue = ((wave.NR34->Period << 8) | *wave.NR33) & 0x7FF;
	}
	else if (addr == 0xFF1E)	// NR34 - Channel 3 period high & control
	{
		*wave.NR34 = data;

		// Check if channel 3 should be turned on
		if (wave.NR34->Trigger == 1)
		{
			wave.Mute = false;
			wave.LenCounterOn = false;
			wave.PatternInd = 0;
			NR52->bCH3 = 1;
		}

		// Update upper 3 bits of period value
		wave.PeriodValue = ((wave.NR34->Period << 8) | *wave.NR33) & 0x7FF;
	}
	else
	{
		gb->RAM[addr] = data;
	}
}