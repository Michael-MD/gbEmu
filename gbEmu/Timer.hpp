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
	uint16_t Counter;
	uint16_t RateBitSelect; // Selects bit from Counter for incrementing timer

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

private:
	bool DelayedBit = 0;
};