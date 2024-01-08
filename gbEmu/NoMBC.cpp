#include "NoMBC.hpp"
#include <fstream>

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
		throw std::invalid_argument(".gb File Not Found.");
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
