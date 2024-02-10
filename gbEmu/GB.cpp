#include "GB.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>

GB::GB(std::string gbFilename)
{
	nClockCycles = 0;

	// Connect SM83 to remainder of system
	cpu.connectGB(this);

	// Insert Cartridge
	cart = new Cartridge(gbFilename);

	// Initialize ppu
	ppu.connectGB(this);

	// Connect Timer Unit
	timer.connectGB(this);

	// ============== Initilizes Registers ==============
	// CPU Internal Registers
	cpu.AF = 0x01B0;
	cpu.BC = 0x0013;
	cpu.DE = 0x00D8;
	cpu.HL = 0x014D;
	cpu.SP = 0xFFFE;
	cpu.PC = 0x0100;

	// PPU Internal Registers
	*P1 = 0xCF;
	*ppu.LCDC = 0x91;
	*ppu.STAT = 0x85;
	*ppu.LY = 153;	// End of vertical blanking
	*ppu.SCY = 0x00;
	*ppu.SCX = 0x00;
	*ppu.BGP = 0xFC;
	*IE = 0x00;
	*IF = 0xE1;

	// Timer Internal Registers
	*timer.TIMA = 0x00;
	*timer.TMA = 0x00;
	*timer.TAC = 0xF8;

	// TODO: Finish remaining initialization

	// ============== Start Game Loop ==============
	gameLoop();
}

void GB::clock()
{
	cpu.clock();
	ppu.clock();
	timer.clock();

	nClockCycles++;
}

uint8_t GB::read(uint16_t addr)
{
	if (addr < 0x8000)		// Cartridge
	{
		return cart->read(addr);
	}
	else if (addr >= 0xA000 && addr < 0xC000)
	{
		return cart->read(addr);
	}

	return RAM[addr];
}

void GB::write(uint16_t addr, uint8_t data)
{
	if (addr < 0x8000)		// Cartridge
	{
		cart->write(addr, data);
	}
	else if (addr >= 0xA000 && addr < 0xC000)
	{
		cart->write(addr, data);
	}
	else if (addr >= 0xC000 && addr <= 0xDE00)
	{
		// Mirror 8kiB internal RAM
		RAM[addr] = data;
		RAM[0xE000 + addr % 0xC000] = data;
	}
	else if (addr >= 0xE000 && addr <= 0xFE00)
	{
		// Mirror 8kiB internal RAM
		RAM[0xC000 + (addr % 0xE000)] = data;
		RAM[addr] = data;
	}
	else if (addr == 0xFF02)	// Serial I/O
	{
		*SC = data;
		if (SC->reg == 0x81)
		{
			SerialOut.push_back(*SB);
			SC->TransferEnable = 0;
		}
	}
	else if (addr == 0xFF04)	// Divider Register
	{
		// Writing any value to Divider register sets it to 0x00.
		*timer.DIV = 0x00;
		timer.Counter = 0x0000;

		// Resetting the timer may have just triggered
		// a falling edge.
		timer.incrementTimer();
	}
	else if (addr == 0xFF05) // TIMA Register
	{
		if (timer.Overflowed)
		{
			// If TIMA is written to during 
			// the four cycles after an overflow
			// then the value written is latched
			// immediately and all subsequent
			// behaviour following an overflow
			// is neglected.
			*timer.TIMA = data;
			timer.Overflowed = false;
		}
		else if (timer.FourClockCyclesB > 0)
		{
			// Ignore writes to TIMA during this period
		}
		else
		{
			*timer.TIMA = data;
		}
	}
	else if (addr == 0xFF06) // TMA Register
	{
		*timer.TMA = data;
		if (timer.FourClockCyclesB > 0)
		{
			*timer.TIMA = *timer.TMA;
		}
	}
	else if (addr == 0xFF07)	// TAC
	{
		// TODO: Writing to TAC obscure behaviour
		*timer.TAC = data;

		// Set rate at which clock is incremented
		switch (timer.TAC->InputClockSelect)
		{
		case 0b00:
			timer.RateBitSelect = 9; // 4096 Hz
			break;
		case 0b01:
			timer.RateBitSelect = 3; // 262144 Hz
			break;
		case 0b10:
			timer.RateBitSelect = 5; // 65536 Hz
			break;
		case 0b11:
			timer.RateBitSelect = 7; // 16384 Hz
			break;
		}
	}
	else if (addr == 0xFF0F)	// IF
	{
		IF->reg = data;
		IF->reg |= 0xE0;
	}
	else if (addr == 0xFF40)	// LCDC Register
	{
		ppu.LCDC->reg = data;
		// Reset PPU when LCD is turned off
		if (ppu.LCDC->bLCDC == 0)
		{
			ppu.reset();
		}
	}
	else if (addr == 0xFF41)	// STAT Register
	{
		// Lower 3-bits are read only
		*ppu.STAT = (data & 0xF8) | (ppu.STAT->reg & 0x07);

		// Writing to this register resets the match flag
		//ppu.STAT->MatchFlag = 0;
	}
	else if (addr == 0xFFFF)	// IE
	{
		IE->reg = data;
		IE->reg |= 0xE0;
	}
	else
	{
		RAM[addr] = data;
	}
}
