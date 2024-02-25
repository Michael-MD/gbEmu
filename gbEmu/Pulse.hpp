#pragma once
#include "Divider.hpp"
#include "SoundChannel.hpp"

#include <cstdint>

class Pulse : public SoundChannel
{
public:
	Pulse();
	~Pulse();

	virtual int8_t GetSample() override;
	void clock();

	// Dividers
	Divider<uint16_t>* PeriodDiv;

	// Length Counter
	bool LenCounterOn = false;
	uint8_t LenCount;

	// Sweep
	// Latches sweep at the start of sweep
	bool SweepOn = false;
	uint8_t CurrentPace = 0;
	// Keeps track of how the pace
	uint8_t SweepEntrances = 0;

	// Envelope
	bool EnvelopeOn = false;
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
	} *NR10;

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
	} *NR11;

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
	} *NR12;

	// Specifies low byte of period
	uint8_t* NR13;

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
	} *NR14;

	// Keeps track of the period which is 
	// broken up between registers NR13 and NR14.
	uint16_t PeriodValue;

private:

};

