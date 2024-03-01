#pragma once
#include <cstdint>
#include "Divider.hpp"

class GBInternal;

class SoundChannel
{
public:
	GBInternal* gb;

	SoundChannel(uint8_t ChannelNum);
	~SoundChannel();

	void connectGB(GBInternal* gb);
	virtual uint8_t GetSample() = 0;
	virtual void clock() = 0;

	// Contains series of events to occur on
	// channel triggering.
	virtual void trigger() = 0;

	// Default volume will be 7.
	// Volume should always remain within the range
	// 0 to 16 (inclusive).
	uint8_t Volume;

	// Keeps track of whether the channel
	// is musted or not for whatever reason.
	bool Mute;

	// Length Counter
	bool LenCounterOn;
	uint8_t LenCount;

	// Sweep
	// Latches sweep at the start of sweep
	bool SweepOn;
	uint8_t CurrentPace = 0;
	// Keeps track of how the pace
	uint8_t SweepEntrances = 0;

	// Envelope
	bool EnvelopeOn;
	// Keeps track of how the pace
	uint8_t EnvelopeEntrances = 0;

	// Dividers
	Divider<uint16_t>* PeriodDiv;

	// Keeps track of the period which is 
	// broken up between registers NRx3 and NRx4.
	uint16_t PeriodValue;

	// Keeps track of the DAC
	bool DACon;

	// Each channel is assigned a numerical value
	uint8_t ChannelNum;
};

