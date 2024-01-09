#include "MBC.hpp"

MBC::MBC(uint8_t ROMSize, uint8_t RAMSize)
{
	// Convert sizes in game header to proper size in bytes

	switch (ROMSize)
	{
	case 0x00:
		nROMBanks = 2;
		break;
	case 0x01:
		nROMBanks = 4;
		break;
	case 0x02:
		nROMBanks = 8;
		break;
	case 0x03:
		nROMBanks = 16;
		break;
	case 0x04:
		nROMBanks = 32;
		break;
	case 0x05:
		nROMBanks = 64;
		break;
	case 0x06:
		nROMBanks = 128;
		break;
	case 0x52:
		nROMBanks = 72;
		break;
	case 0x53:
		nROMBanks = 80;
		break;
	case 0x54:
		nROMBanks = 96;
		break;
	}

	for (int p = 0; (nROMBanks - 1) >> p++ != 0; Log2nROMBanks = p);
	ROMSizeBytes = ROM_BANK_SIZE * nROMBanks;

	switch (RAMSize)
	{
	case 0x00:
		nRAMBanks = 0;
		break;
	case 0x01:
	case 0x02:
		nRAMBanks = 1;
		break;
	case 0x03:
		nRAMBanks = 4;
		break;
	case 0x04:
		nRAMBanks = 16;
		break;
	}

	for (int p = 0; (nRAMBanks - 1) >> p++ != 0; Log2nRAMBanks = p);
	RAMSizeBytes = RAM_BANK_SIZE * nRAMBanks;

	ROM = new uint8_t[ROMSizeBytes];
	RAM = new uint8_t[RAMSizeBytes];

}

MBC::~MBC()
{
	delete[] ROM;
	delete[] RAM;
}