#include "Cartridge.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#include "GBInternal.hpp"
#include "NoMBC.hpp"
#include "MBC1.hpp"
#include "MBC2.hpp"
#include "MBC3.hpp"

Cartridge::Cartridge(std::string gbFilename)
{
	// Load gb Cartridge into Data Structure
	std::ifstream ifs;
	ifs.open(gbFilename, std::ifstream::binary);

	if (ifs.is_open())
	{
		// Get Game Title
		ifs.seekg(0x134, std::ios::beg);
		ifs.read(GameTitle, 16);
		GameTitle[16] = '\n';

		// Get Game Header Information
		ifs.seekg(0x143, std::ios::beg);
		ifs.read(reinterpret_cast<char*>(Header), sizeof(*Header));

		ifs.close();
	}
	else
	{
		std::cout << ".gb file not found." << std::endl;
		std::exit(1);
	}

	// Emulation Info
	switch (Header->CartType)
	{
	case 0x00:
		mbc = new NoMBC(gbFilename, Header->ROMSize, Header->RAMSize);
		break;
	case 0x01:
	case 0x02:
	case 0x03:
		mbc = new MBC1(gbFilename, Header->ROMSize, Header->RAMSize);
		break;
	case 0x05:
	case 0x06:
		mbc = new MBC2(gbFilename, Header->ROMSize, Header->RAMSize);
		break;
	case 0x012:
	case 0x013:
		mbc = new MBC3(gbFilename, Header->ROMSize, Header->RAMSize);
		break;
	default:
		std::cout << "Only Cartridges which use ROM only are supported."
			<< "The inserted cartridge type is 0x"
			<< std::hex << (int)Header->CartType << ".";
		std::exit(1);
	}


	// Display some information about the game
	std::cout << "Title: " << GameTitle << std::endl;

	std::cout << "Cartridge Type: " << (int)Header->CartType << std::endl;

	std::cout << "License Code: " 
		<< ((Header->LicenseCodeH << 8) | Header->LicenseCodeL) << std::endl;

	std::cout << "Destination Code: " 
		<< (Header->DestinationCode == 0 ? "Japanese" : "Non-Japanese") << std::endl;
}

void Cartridge::write(uint16_t addr, uint8_t data)
{
	mbc->write(addr, data);
}

uint8_t Cartridge::read(uint16_t addr)
{
	return mbc->read(addr);
}

Cartridge::~Cartridge()
{
	delete mbc;
}