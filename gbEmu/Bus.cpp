#include "Bus.hpp"

uint8_t Bus::read(uint16_t addr)
{
	if (addr >= 0x0000 && addr < 0x4000)		// 16kB ROM bank #0 
	{

	}
	else if (addr >= 0x4000 && addr < 0x8000)	// 16kB switchable ROM bank
	{

	}
	else if (addr >= 0x8000 && addr < 0xA000)	// 8kB Video RAM
	{

	}
	else if (addr >= 0xA000 && addr < 0xC000)	// 8kB switchable RAM bank
	{

	}
	else if (addr >= 0xC000 && addr < 0xE000 || addr >= 0xE000 && addr < 0xFE00)	// 8kB Internal RAM
	{

	}
	else if (addr >= 0xFE00 && addr < 0xFEA0)	// Sprite Attrib Memory (OAM)
	{

	}
	else if (addr >= 0xFEA0 && addr < 0xFF00)	// Empty but unusable for I/O
	{

	}
	else if (addr >= 0xFF00 && addr < 0xFF4C)	// I/O ports
	{

	}
	else if (addr >= 0xFF4C && addr < 0xFF80)	// Empty but unusable for I/O
	{

	}
	else if (addr >= 0xFF80 && addr < 0xFFFF)	// Internal RAM
	{

	}
	else if (addr == 0xFFFF)					// Interrupt Enable Register
	{

	}

	return 0x00;
}

void Bus::write(uint16_t addr, uint8_t data)
{

}