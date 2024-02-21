#include "MBC2.hpp"

MBC2::MBC2(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize) : MBC(gbFilename, ROMSize, RAMSize)
{
	// Initialize internal registers
	ROMBankCode = 0x01;
	RAMEnable = false;
}

void MBC2::write(uint16_t addr, uint8_t data)
{
	if (addr >= 0x0000 && addr < 0x4000)	// RAM enable
	{
		if (((addr >> 8) & 1) == 0)	// Controls RAM enable
		{
			// Write to this address space 0xXA 
			// to enables RAM.
			RAMEnable = (data & 0x0F) == 0xA;
		}
		else	// Controls ROM bank
		{
			if ((data & 0x1F) == 0)
			{
				ROMBankCode = 0x01;
			}
			else
			{
				ROMBankCode = data & 0x0F;
			}
		}
	}
	else if (addr >= 0xA000 && addr < 0xC000) // RAM write
	{
		if (RAMEnable)
		{
			RAM[addr % 0x0200] = data;
		}
	}
}

uint8_t MBC2::read(uint16_t addr)
{
	if (addr >= 0x0000 && addr < 0x4000)	// Lower 16kiB ROM bank
	{
		return ROM[addr];
	}
	else if (addr >= 0x4000 && addr < 0x8000) // Upper 16kiB ROM bank
	{
		// Here the upper bank select is always used and we can switch 
		// freely between banks.
		return ROM[ROMBankCode * 0x4000 + (addr % 0x4000)];
	}
	else if (addr >= 0xA000 && addr < 0xC000) // RAM read
	{
		if (RAMEnable)
		{
			// Only the first 9-bits are read.
			// This results in 0xA200–0xBFFF 
			// echoing 0xA000–0xA1FF.
			return RAM[addr % 0x0200];
		}
		else
		{
			// Open bus behaviour
			return 0xFF;
		}
	}

	return 0x00;
}