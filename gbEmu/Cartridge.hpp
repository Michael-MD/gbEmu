#pragma once
#include <cstdint>
#include <string>
#include <cstring>
#include "MBC.hpp"

class Cartridge
{
public:
	Cartridge(std::string gbFilename);
	~Cartridge();

	MBC *mbc;

	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	// Emulation Info
	char GameTitle[16 + 1];

	struct
	{
		uint8_t GBColor;	// 0x143
		// $80 = Color GB 
		// $00 or other = not Color GB
		uint8_t LicenseCodeH; // 0x144
		uint8_t LicenseCodeL; // 0x145
		uint8_t SuperGB;	// 0x146, 
		// 00 = Gameboy
		// 0x03 = Super GB/SGB functions
		uint8_t CartType;
		// 0 - ROM ONLY						12 - ROM + MBC3 + RAM
		// 1 - ROM + MBC1					13 - ROM + MBC3 + RAM + BATT
		// 2 - ROM + MBC1 + RAM				19 - ROM + MBC5
		// 3 - ROM + MBC1 + RAM + BATT		1A - ROM + MBC5 + RAM
		// 5 - ROM + MBC2					1B - ROM + MBC5 + RAM + BATT
		// 6 - ROM + MBC2 + BATTERY			1C - ROM + MBC5 + RUMBLE
		// 8 - ROM + RAM					1D - ROM + MBC5 + RUMBLE + SRAM
		// 9 - ROM + RAM + BATTERY			1E - ROM + MBC5 + RUMBLE + SRAM + BATT
		// B - ROM + MMM01					1F - Pocket Camera
		// C - ROM + MMM01 + SRAM			FD - Bandai TAMA5
		// D - ROM + MMM01 + SRAM + BATT	FE - Hudson HuC - 3
		// F - ROM + MBC3 + TIMER + BATT	FF - Hudson HuC - 1
		// 10 - ROM + MBC3 + TIMER + RAM + BATT
		// 11 - ROM + MBC3
		uint8_t ROMSize;	// 0x148
		// 0 - 256Kbit = 32KByte = 2 banks
		// 1 - 512Kbit = 64KByte = 4 banks
		// 2 - 1Mbit = 128KByte = 8 banks
		// 3 - 2Mbit = 256KByte = 16 banks
		// 4 - 4Mbit = 512KByte = 32 banks
		// 5 - 8Mbit = 1MByte = 64 banks
		// 6 - 16Mbit = 2MByte = 128 banks
		// $52 - 9Mbit = 1.1MByte = 72 banks
		// $53 - 10Mbit = 1.2MByte = 80 banks
		// $54 - 12Mbit = 1.5MByte = 96 banks
		uint8_t RAMSize;	// 0x149
		// 0 - None
		// 1 - 16kBit = 2kB = 1 bank
		// 2 - 64kBit = 8kB = 1 bank
		// 3 - 256kBit = 32kB = 4 banks
		// 4 - 1MBit = 128kB = 16 banks
		uint8_t DestinationCode;	// 0x14A
		// 0 - Japanese 
		// 1 - Non-Japanese
		uint8_t LincenceCode;	// 0x14B
		uint8_t MaskROMVersionNum;
		uint8_t ComplementCheck;
		uint8_t ChecksumH;
		uint8_t ChecksumL;
	} *Header = reinterpret_cast<decltype(Header)>(ROM + 0x143);

private:
	uint8_t ROM[0xFFFF + 1];
	
};