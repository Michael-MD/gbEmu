#include "Bus.hpp"
#include <fstream>
#include <stdexcept>

Bus::Bus(std::string gbFilename)
{
	nClockCycles = 0;
	*LY = 0;

	// Connect CPU to remainder of system
	cpu.bus = this;

	// Load gb Cartridge into Data Structure
	std::ifstream ifs;
	ifs.open(gbFilename, std::ifstream::binary);

	if (ifs.is_open())
	{
		ifs.read((char*)RAM, sizeof(RAM));
		ifs.close();
	}
	else
	{
		throw std::invalid_argument(".gb File Not Found.");
	}

	// At Power Up, a 256-byte program is executed starting at 0
	cpu.PC = 0x0000;

	// ============== Initilizes Registers ==============
	*TMA = 0x00;
	*TAC = 0x00;
	*P1 = 0x00;
	//bus->write(0xFF10, 0x80);  // NR10
	//bus->write(0xFF11, 0xBF);  // NR11
	//bus->write(0xFF12, 0xF3);  // NR12
	//bus->write(0xFF14, 0xBF);  // NR14
	//bus->write(0xFF16, 0x3F);  // NR21
	//bus->write(0xFF17, 0x00);  // NR22
	//bus->write(0xFF19, 0xBF);  // NR24
	//bus->write(0xFF1A, 0x7F);  // NR30
	//bus->write(0xFF1B, 0xFF);  // NR31
	//bus->write(0xFF1C, 0x9F);  // NR32
	//bus->write(0xFF1E, 0xBF);  // NR33
	//bus->write(0xFF20, 0xFF);  // NR41
	//bus->write(0xFF21, 0x00);  // NR42
	//bus->write(0xFF22, 0x00);  // NR43
	//bus->write(0xFF23, 0xBF);  // NR30
	//bus->write(0xFF24, 0x77);  // NR50
	//bus->write(0xFF25, 0xF3);  // NR51
	//bus->write(0xFF26, 0xF1);  // NR52
	//*LCDC = 0x91;
	*SCY = 0x00;
	*SCX = 0x00;
	//bus->write(0xFF45, 0x00);  // LYC
	//bus->write(0xFF47, 0xFC);  // BGP
	//bus->write(0xFF48, 0xFF);  // OBP0
	//bus->write(0xFF49, 0xFF);  // OBP1
	//bus->write(0xFF4A, 0x00);  // WY
	//bus->write(0xFF4B, 0x00);  // WX
	*IE = 0x00;
}


void Bus::clock()
{
	// The system should be clocked in DMG mode
	// at 4f where f=4.1943MHz i.e. machine cycles.

	nClockCycles++;

	cpu.clock();

	// Increment Divider register at 8.192kHz.
	if (nClockCycles % 0xFF == 0 && nClockCycles % 4 == 0)
		(*Div)++;

	// ============= LCD Display ============= 
	if (nClockCycles % 1890 == 0) // Do Entire Row at Once
	{
		// TODO: SCX

		uint8_t BGStartAddr = LCDC->BGCodeArea ? 0x9C00 : 0x9800;

		for (int ColBlock = 0; ColBlock < 20; ColBlock++)
		{
			// Get CHR Code
			uint8_t CHRCode = RAM[BGStartAddr + *LY * 20 + ColBlock];

			// Find Corresponding Tile
			uint8_t DotDataAddr = (CHRCode < 0x80 ? 0x9000 : 0x8800) + 0x0F * CHRCode;

			// Parse Dot Data
			uint8_t TileLO = RAM[DotDataAddr + 16 * *LY + 0];
			uint8_t TileHI = RAM[DotDataAddr + 16 * *LY + 1];

			// Get pixel Shade for entire row of pixels
			for (int p = 0; p < 8; p++)
			{
				uint8_t PixelPalette = ((TileHI & (1 << p)) >> (p - 1)) | (TileLO & (1 << p)) >> p;

				// Store Result Display Grid
				Display[*LY][ColBlock + p] = (*BGP >> (PixelPalette * 2)) & 0b11;
			}
		}

		// Check if in Vertical Blanking Period
		if (*LY == 17)
		{
			IF->VerticalBlanking = 1;
		}
		else if (*LY == (17 + 10))
		{
			IF->VerticalBlanking = 0;
		}

		*LY++;
		
	}


	// TODO: window display

}

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
	else if (addr >= 0xC000 && addr < 0xFE00)	// 8kB Internal RAM
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

	return RAM[addr];
}

void Bus::write(uint16_t addr, uint8_t data)
{
	if (addr >= 0xC000 && addr < 0xFE00)	// 8kB Internal RAM
	{
		// Echo 8kB Internal RAM

		if (addr >= 0xC000 && addr < 0xE000)
		{
			RAM[addr] = data;
			RAM[0xE000 + addr & 0xC000] = data;
		}
		else
		{
			RAM[0xC000 + addr & 0xE000] = data;
			RAM[addr] = data;
		}
	}
	else if (addr == 0xFF04)	// Divider Register
	{
		// Writing any value to Divider register sets it to 0x00.
		*Div = 0x00;
	}
	else
	{
		RAM[addr] = data;
	}
}
