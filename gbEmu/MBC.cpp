#include "MBC.hpp"
#include <fstream>
#include <iostream>

MBC::MBC(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize)
{
	// Convert sizes in game header to proper size in bytes
	// and create ROM and RAM on heap since these could be up
	// to several MiB.

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
	case 0x07:
		nROMBanks = 256;
		break;
	case 0x08:
		nROMBanks = 512;
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

	if (nROMBanks != 0)
		for (int p = 0; ((nROMBanks - 1) >> p++) != 0; Log2nROMBanks = p);
	else Log2nROMBanks;

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
	case 0x05:
		nRAMBanks = 8;
		break;
	}

	if (nRAMBanks != 0)
		for (int p = 0; ((nRAMBanks - 1) >> p++) != 0; Log2nRAMBanks = p);
	else Log2nRAMBanks;

	RAMSizeBytes = RAM_BANK_SIZE * nRAMBanks;

	ROM = new uint8_t[ROMSizeBytes];
	RAM = new uint8_t[RAMSizeBytes];

	// Populate ROM with cartridge data
	std::ifstream ifs;
	ifs.open(gbFilename, std::ifstream::binary);

	if (ifs.is_open())
	{
		ifs.read(reinterpret_cast<char*>(ROM), ROMSizeBytes);
		ifs.close();

		// TODO: RAM
	}
	else
	{
		std::cout << ".gb file not found." << std::endl;
		std::exit(1);
	}

}

MBC::~MBC()
{
	delete[] ROM;
	delete[] RAM;
}