#include "Timer.hpp"
#include "GB.hpp"

void Timer::connectGB(GB* gb)
{
	this->gb = gb;

	// Divider (Read/Reset)
	DIV = gb->RAM + 0xFF04;

	// TIMA Register
	TIMA = gb->RAM + 0xFF05;

	// TMA Register
	TMA = gb->RAM + 0xFF06;

	// TAC Register
	TAC = reinterpret_cast<decltype(TAC)>(gb->RAM + 0xFF07);

	// Default Register Values
	*TIMA = 0x00;
	*TMA = 0x00;
	*TAC = 0x00;
}

void Timer::clock()
{
	// TODO: obscure behaviour
	
	// Increment TIMA register, if the current
	// value is 0xFF then the register will overflow
	// and we load TMA into TIMA and set the inerrupt
	// request bit in register IF.

	if(TAC->Enable && gb->nClockCycles % TickRate == 0)
	{
		if (*TIMA == 0xFF)
		{
			*TIMA = *TMA;
			gb->IF->TimerOverflow = 1;
		}
		else
		{
			(*TIMA)++;
		}
	}

	// Increment DIV Register
	// DIV is incremented at 16384Hz (~16779Hz on 
	if (gb->nClockCycles % 256 == 0) (*DIV)++;
}