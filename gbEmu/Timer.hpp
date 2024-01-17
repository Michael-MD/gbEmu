#pragma once
#include <cstdint>

class GB;

class Timer
{
public:
	GB* gb;

	void connectGB(GB* gb);
	void clock();

	int TickRate;

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
};