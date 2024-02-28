#pragma once
#include "SoundChannel.hpp"

#include <cstdint>

class Wave : public SoundChannel
{
public:
	Wave(uint8_t ChannelNum);
	~Wave();

	virtual uint8_t GetSample() override;
	void clock();

	// Registers

	// NR30: Channel 3 DAC enable
	union NR30Register	
	{
		struct 
		{
			uint8_t _ : 7;
			uint8_t bDAC : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR30;


	// NR31: Channel 3 length timer [write-only]
	uint8_t* NR31;


	// NR32: Channel 3 output level
	union NR32Register	
	{
		struct 
		{
			uint8_t _ : 5;
			uint8_t OutLvl : 2;
			uint8_t __ : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR32;

	// NR33: Channel 3 period low [write-only]
	uint8_t* NR33;

	// NR34: Channel 3 period high & control
	union NR34Register
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
	} *NR34;

	// Pattern RAM
	uint8_t *PatternRAM;

	// Keeps track of the byte in
	// pattern memeory to be played next.
	// This variable should always be within
	// 0-31.
	uint8_t PatternInd = 0;

private:

};

