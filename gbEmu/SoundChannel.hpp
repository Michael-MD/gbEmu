#pragma once
#include <cstdint>
#include "Divider.hpp"

class GB;

class SoundChannel
{
public:
	GB* gb;

	void connectGB(GB* gb);
	virtual int8_t GetSample() = 0;

	// Default volume will be 0.
	// Volume should always remain within the range
	// 0 to 15 (inclusive).
	uint8_t Volume;

	// Keeps track of whether the channel
	// is musted or not for whatever reason.
	bool Mute = false;

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

	// Sweep
	union NR10Register
	{
		struct
		{
			uint8_t Step : 3;
			uint8_t Direction : 1;
			uint8_t Pace : 3;
			uint8_t _ : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx0;

	// Length Timer and Duty Cycle
	union NR11Register
	{
		struct
		{
			uint8_t InitLenTimer : 6;
			uint8_t Duty : 2;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx1;

	// Volume and Envelope
	union NR12Register
	{
		struct
		{
			uint8_t SweepPace : 3;
			uint8_t EnvDir : 1;
			uint8_t InitVol : 4;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx2;

	// Specifies low byte of period
	uint8_t* NRx3;

	// Period High and Control
	union NR14Register
	{
		struct
		{
			uint8_t Period : 3;
			uint8_t _ : 3;
			uint8_t LenEnable : 1;
			uint8_t Trigger : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx4;
};

