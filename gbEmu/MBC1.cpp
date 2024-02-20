#include "MBC1.hpp"
#include <fstream>
#include <iostream>

MBC1::MBC1(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize) : MBC(gbFilename, ROMSize, RAMSize)
{
	// Initialize internal registers
	ROMBankCode = 0x01;
	UpperROMBankCode = 0x00;
	bBankingMode = 0x00;
	RAMEnable = 0;

	ROMMask = (1 << Log2nROMBanks) - 1;
	RAMMask = (1 << Log2nRAMBanks) - 1;
}

void MBC1::write(uint16_t addr, uint8_t data)
{
	if (addr >= 0x0000 && addr < 0x2000)	// RAM enable
	{
		// Write to this address space 0xXA 
		// to enables RAM.
		RAMEnable = (data & 0x0F) == 0xA;
	}
	else if (addr >= 0x2000 && addr < 0x4000)	// ROM bank select
	{
		ROMBankCode = data & 0x1F;
		// Select Bank
		if (ROMBankCode == 0)
		{
			// 00->01 bank translation
			ROMBankCode = 1;
		}

		// Mask unncessary bits
		ROMBankCode &= ROMMask;
	}
	else if (addr >= 0x4000 && addr < 0x6000)	// Upper ROM bank select / RAM bank select
	{
		UpperROMBankCode = data & 0b11;
	}
	else if (addr >= 0x6000 && addr < 0x8000)	// Banking mode select
	{
		bBankingMode = data & 0x01;
	}
}

uint8_t MBC1::read(uint16_t addr)
{
	if (addr >= 0x0000 && addr < 0x4000)	// Lower 16kiB ROM bank
	{
		if (bBankingMode == 0)
		{
			// Access 0-th 16kiB bank
			return ROM[addr];
		}
		else
		{
			// Allow for secondary banking ROM (16kiB) to select larger registers 
			// with the first 5 bits still 0. If the ROM doesn't
			// have enough masks then this effect is unnoticeable.
			return ROM[((UpperROMBankCode << 5) & ROMMask) * 0x4000 + addr];
		}
	}
	else if (addr >= 0x4000 && addr < 0x8000) // Upper 16kiB ROM bank
	{
		// Here the upper bank select is always used and we can switch 
		// freely between banks.
		return ROM[(((UpperROMBankCode << 5) | ROMBankCode) & ROMMask) * 0x4000 + (addr % 0x4000)];
	}
	else if (addr >= 0xA000 && addr < 0xC000) // RAM read
	{
		if (RAMEnable)
		{
			if (bBankingMode == 0)
			{
				RAM[addr % 0xA000];
			}
			else
			{
				// Access any of up to 4 8kiB RAM banks
				RAM[((UpperROMBankCode & RAMMask) * 0x2000) + (addr % 0xA000)];
			}
		}
		else
		{
			// Open bus behaviour
			return 0xFF;
		}
	}

	return 0x00;
}
