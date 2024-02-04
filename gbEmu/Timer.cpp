#include "Timer.hpp"
#include "GB.hpp"

void Timer::connectGB(GB* gb)
{
	this->gb = gb;

	// Divider (Read/Reset)
	DIV = gb->RAM + 0xFF04;
	Counter = 0xAB00;
	*DIV = Counter >> 8;

	// TIMA Register
	TIMA = gb->RAM + 0xFF05;

	// TMA Register
	TMA = gb->RAM + 0xFF06;

	// TAC Register
	TAC = reinterpret_cast<decltype(TAC)>(gb->RAM + 0xFF07);
}

void Timer::clock()
{

	// There is an internal counter which 
	// increments at the clock frequency.
	// This is used for the divider register 
	// which is in turn used for the timing
	// circuit.
	Counter++;

	// Increment DIV Register
	// DIV is incremented at 16384Hz (~16779Hz on DMG)
	(*DIV) = Counter >> 8;	// DIV register is upper 8 bits of internal counter

	incrementTimer();


	if (Overflowed)
	{
		if (FourClockCyclesA > 0)
		{
			FourClockCyclesA--;
		}
		else
		{
			gb->IF->TimerOverflow = 1;
			*TIMA = *TMA;
			Overflowed = false;

			// Start counting down the next 4 clock cycles
			FourClockCyclesB = 4;
		}
	}

	if (FourClockCyclesB > 0)
	{
		FourClockCyclesB--;
	}
}

void Timer::incrementTimer()
{
	// Increment TIMA register using DIV, if the current
	// value is 0xFF then the register will overflow
	// and we load TMA into TIMA and set the inerrupt
	// request bit in register IF.

	// Select appropriate bit from Counter. If timer is 
	// disabled then mux result is reset.
	bool CurrentCounterBit = (Counter >> RateBitSelect) & TAC->Enable;

	// Falling edge detector:
	// If previous bit was 1 and
	// current bit is 0 then we have just
	// encountered a falling edge and we
	// should increment the timer. 
	if (DelayedBit && !CurrentCounterBit)
	{
		// If an overflow occurs, the effects are delayed
		// by a 4 clock cycles.
		if (*TIMA == 0xFF)
		{
			*TIMA = 0x00; // TIMA is set to 0 for 4 clock cycles.
			Overflowed = true;
			FourClockCyclesA = 4;
		}
		else
		{
			(*TIMA)++;
		}
	}

	DelayedBit = CurrentCounterBit;
}