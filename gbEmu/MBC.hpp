#pragma once
#include <cstdint>

class MBC
{
public:
	MBC(uint8_t ROMSize, uint8_t RAMSize);
	~MBC();

	uint8_t* ROM;
	uint8_t* RAM;

	const int ROM_BANK_SIZE = 16 * 1024;
	const int RAM_BANK_SIZE = 8 * 1024;
	int nROMBanks, nRAMBanks;
	int Log2nROMBanks, Log2nRAMBanks;
	int ROMSizeBytes, RAMSizeBytes;

	virtual void write(uint16_t addr, uint8_t data) = 0;
	virtual uint8_t read(uint16_t addr) = 0;
};