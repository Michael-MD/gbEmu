#pragma once
#include "SoundChannel.hpp"

#include <cstdint>

class Noise : public SoundChannel
{
public:
	Noise(uint8_t ChannelNum);
	~Noise();

	virtual uint8_t GetSample() override;
	void clock();
	void trigger() override;

	// Registers

	union NR41Register
	{
		struct
		{
			uint8_t InitLenTimer : 6;
			uint8_t _ : 2;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR41;

	// Volume and Envelope
	union NR42Register
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
	} *NR42;

	union NR43Register
	{
		struct
		{
			uint8_t ClockDiv : 3;
			uint8_t LFSRWidth : 1;
			uint8_t ClockShift : 4;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR43;

	union NR44Register
	{
		struct
		{
			uint8_t _ : 6;
			uint8_t LenEnable : 1;
			uint8_t Trigger : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR44;

	// LFSR for generating white noise
	union
	{
		struct
		{
			uint16_t Bit0 : 1;
			uint16_t Bit1 : 1;
			uint16_t _ : 5;
			uint16_t Bit7 : 1;
			uint16_t __ : 7;
			uint16_t Bit15 : 1;
		};

		uint16_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} LFSR;

	uint8_t ClockEntrances = 0;
};

