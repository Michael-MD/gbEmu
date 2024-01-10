#include "MBC1.hpp"
#include <fstream>
#include <iostream>

MBC1::MBC1(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize) : MBC(ROMSize, RAMSize)
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

	// Initialize internal registers
	ROMBankCode = 0x00;
	UpperROMBankCode = 0x00;
	bBankingMode = 0x00;
	bRAMEnable = 0;
}

void MBC1::write(uint16_t addr, uint8_t data)
{
	// TODO
}

uint8_t MBC1::read(uint16_t addr)
{
	// ROM
	if (addr < 0x4000)
	{
		if (bBankingMode == 0 || nROMBanks < 64)
		{
			// In banking mode 0, this address space is
			// always mapped to bank 0 of ROM or
			// if less than 1MiB is being mapped then regardless
			// of the bank mapping mode this address range always 
			// points to bank 0 of ROM.
			return ROM[addr];
		}
		else
		{
			// In banking mode 1, there is now the possibility
			// that if the MBC is mapping more than 1MiB
			// then if the upper banking mode is set 1 one then
			// ROM banks 0x20/0x40/0x60 can now be accessed from 
			// this memory space.
			if (nROMBanks >= 64)
			{
				// Using RAM bank register as upper ROM bank select
				// ROM banks are of size 16KiB (0x4000). 
				// 
				// We make the ROM bank number and get the corresponding address.
				// The requested address is then an offset from this starting
				// point.
				uint8_t BankNum = (UpperROMBankCode << 5) | ROMBankCode;
				if(BankNum < nROMBanks)
				{
					return ROM[0x4000 * ((UpperROMBankCode << 5) | ROMBankCode) + addr];
				}
				else
				{
					// If execution makes it here then a bank which  
					// is out of range is being accessed. This should
					// never happen but in case it does the code will simply 
					// return 0x00. This will only occur if the ROM is 1MiB
					// but the addressing for some reason is assuming 2MiB.
					return 0x00;
				}
			}
		}
	}
	else if (addr >= 0x4000 && addr < 0x8000)
	{
		
		if (bBankingMode == 0 || nROMBanks < 64)
		{
			// The banks which could be accessed in 
			// range <0x4000 cannot be mapped to this space.
			// In cases where we map to 0x00/0x20/0x40/0x60
			// we simply select the next register.
			switch (ROMBankCode)
			{
			case 0x00:
			case 0x20:
			case 0x40:
			case 0x60:
				return ROM[0x4000 * (ROMBankCode + 1) + (addr % 0x4000)];
			default:
				return ROM[0x4000 * ROMBankCode + (addr % 0x4000)];
			}
		}
		else
		{
			
			if (nROMBanks >= 64)
			{
				// Using RAM bank register as upper ROM bank select
				// ROM banks are of size 16KiB (0x4000). 
				// 
				// We make the ROM bank number and get the corresponding address.
				// The requested address is then an offset from this starting
				// point.
				uint8_t BankNum = (UpperROMBankCode << 5) | ROMBankCode;
				if (BankNum < nROMBanks)
				{
					switch (ROMBankCode)
					{
					case 0x00:
					case 0x20:
					case 0x40:
					case 0x60:
						return ROM[0x4000 * (BankNum + 1) + (addr % 0x4000)];
					default:
						return ROM[0x4000 * BankNum + (addr % 0x4000)];
					}
				}
				else
				{
					return 0x00;
				}
			}
		}
		
	}

	// RAM
	else if (addr >= 0xA000 && addr < 0xC000)
	{
		if(bRAMEnable)
		{
			if (bBankingMode)
			{
				// The register which selects the RAM
				// bank is used to select the upper two bits
				// of the ROM bank. Hence RAM bank 0 is
				// always referred.
				return RAM[addr % 0xA000];
			}
			else
			{
				return RAM[0x2000 * UpperROMBankCode + (addr % 0xA000)];
			}
		}
		else
		{
			// If a read from RAM is attempted while 
			// reading is disabled, then the system
			// exibits open bus behaviour and we end
			// up with a random value although most of
			// the time the value returned is 0xFF. 
			return 0xFF;
		}

	}

	return 0x00;
}
