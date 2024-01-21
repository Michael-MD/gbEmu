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

	// Initialize ppulay
	ppu.connectGB(this);

	// Connect Timer Unit
	timer.connectGB(this);

	// ============== Initilizes Registers ==============
	// CPU Internal Registers
	cpu.AF = 0x1180;
	cpu.BC = 0x0000;
	cpu.DE = 0xFF56;
	cpu.HL = 0x000D;
	cpu.SP = 0xFFFE;
	cpu.PC = 0x0100;

	*NR10 = 0x80;
	*NR11 = 0xBF;
	*NR12 = 0xF3;
	*NR14 = 0xBF;
	*NR21 = 0x3F;
	*NR22 = 0x00;
	*NR24 = 0xBF;
	*NR30 = 0x7F;
	*NR31 = 0xFF;
	*NR32 = 0x9F;
	*NR33 = 0xBF;
	*NR41 = 0xFF;
	*NR42 = 0x00;
	*NR43 = 0x00;
	*NR44 = 0xBF;
	*NR50 = 0x77;
	*NR51 = 0xF3;
	*NR52 = cart->Header->SuperGB ? 0xF0 : 0xF1;

	*ppu.LCDC = 0x91;
	*P1 = 0x00;
	*ppu.LY = 0;
	*ppu.SCY = 0x00;
	*ppu.SCX = 0x00;
	*ppu.BGP = 0xFC;
	*IE = 0x00;

	// TODO: Finish remaining initialization

	// ============== Start Game Loop ==============
	gameLoop();
}

void GB::clock()
{
	nClockCycles++;

	cpu.clock();
	//ppu.clock();
	//timer.clock();

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
	}
	else if (addr == 0xFF07)	// TAC
	{
		// Set rate at which clock is incremented
		switch (timer.TAC->InputClockSelect)
		{
		case 0b00:
			timer.TickRate = 1024;		// 4096 Hz
			break;
		case 0b01:
			timer.TickRate = 16;		// 262144 Hz
			break;
		case 0b10:
			timer.TickRate = 64;		// 65536 Hz
			break;
		case 0b11:
			timer.TickRate = 256;		// 16384 Hz
			break;
		}
	}
	else
	{
		RAM[addr] = data;
	}
}
