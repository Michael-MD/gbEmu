#pragma once
#include <cstdint>

class GB;

class Timer
{
public:
	GB* gb;

	void connectGB(GB* gb);
	void clock();
	void incrementTimer();

	// Divider (Read/Reset)
	uint8_t* DIV;

	// TIMA Register
	uint8_t* TIMA;

	// TMA Register
	uint8_t* TMA;

	// TAC Register
	union
	{
		struct
		{
			uint8_t InputClockSelect : 2;
			uint8_t Enable : 1;
		};

		uint8_t reg_;

		void operator=(uint8_t reg)
		{
			reg_ = reg;
		};

	} *TAC;

	uint16_t Counter;
	uint16_t RateBitSelect; // Selects bit from Counter for incrementing timer
	bool Overflowed = false; // Indicates overflow has occured
	
	uint8_t FourClockCyclesA = 0;	// Four clock cycles after overflow
	uint8_t FourClockCyclesB = 0;	// Four clock cycles after overflow takes effect.

private:
	bool DelayedBit = 0;
};