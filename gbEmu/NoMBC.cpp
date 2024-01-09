#include "NoMBC.hpp"
#include <fstream>
#include <iostream>

NoMBC::NoMBC(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize) : MBC(ROMSize, RAMSize)
{
	std::ifstream ifs;
	ifs.open(gbFilename, std::ifstream::binary);

	if (ifs.is_open())
	{
		ifs.read(reinterpret_cast<char*>(ROM), ROMSizeBytes);
		ifs.close();
	}
	else
	{
		std::cout << ".gb file not found." << std::endl;
		std::exit(1);
	}
}

void NoMBC::write(uint16_t addr, uint8_t data)
{
	// Cannot write to ROM
}

uint8_t NoMBC::read(uint16_t addr)
{
	if (addr < 0x8000)
	{
		return ROM[addr];
	}

	return 0x00;
}
