#include "MBC3.hpp"


MBC3::MBC3(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize) : MBC(gbFilename, ROMSize, RAMSize)
{
	// Initialize internal registers
	ROMBankCode = 0x01;
	RAMBankCode = 0x00;
	RAMEnable = 0;

	MappingRAM = false;
	ROMMask = (1 << Log2nROMBanks) - 1;
	RisingEdge = 0x00;
	RTCSelect = 0x00;
	HaltTimer = false;

	StartTime = std::chrono::system_clock::now();
}

void MBC3::write(uint16_t addr, uint8_t data)
{
	if (addr >= 0x0000 && addr < 0x2000)	// RAM/RTC enable
	{
		// Write to this address space 0x0A 
		// to enables RAM.
		if (data == 0x0A)
		{
			RAMEnable = 1;
		}
		else if (data == 0x00)
		{
			RAMEnable = 0;
		}
	}
	else if (addr >= 0x2000 && addr < 0x4000)	// ROM bank select
	{
		ROMBankCode = data & 0x7F;
		// Select Bank
		if (ROMBankCode == 0)
		{
			// 00->01 bank translation
			ROMBankCode = 1;
		}

		// Mask unncessary bits
		ROMBankCode &= ROMMask;
	}
	else if (addr >= 0x4000 && addr < 0x6000)	// Upper ROM bank select / RAM bank select
	{
		if (data >= 0 && data <= 0x03)	// RAM bank select
		{
			MappingRAM = true;
			RAMBankCode = data;
		}
		else if (data >= 0x08 && data <= 0x0C)	// RAM bank select
		{
			MappingRAM = false;
			RTCSelect = data % 0x08;
		}
	}
	else if (addr >= 0x6000 && addr < 0x8000)	// Latch clock into RTC
	{
		// On a rising edge 0x00->0x01, the current
		// time is latched into the RTC register.
		if (data == 0x01 && RisingEdge == 0)
		{
			// If the timer is not halted then add the time elapsed
			if(!HaltTimer)
			{
				// Get time elapsed since last write
				auto ElapsedTime = std::chrono::system_clock::now() - StartTime;

				// Convert time stored into chrono object
				std::chrono::seconds RTCTime(RTC[0] * 1 + RTC[1] * 60 + RTC[2] * 3600 + RTC[3] * 86400);

				// Add time elapsed to time in RTC
				auto UpdatedTime = RTCTime + ElapsedTime;

				RTC[0] = std::chrono::duration_cast<std::chrono::seconds>(UpdatedTime).count() % 60; // Seconds 0-59
				RTC[1] = std::chrono::duration_cast<std::chrono::minutes>(UpdatedTime).count() % 60; // Minutes 0-59
				RTC[2] = std::chrono::duration_cast<std::chrono::hours>(UpdatedTime).count(); // Hours 0-23
				RTC[2] %= 24;
				RTC[3] /= 24;	// days 0x00-0xFF

				// Since I will not implement save functionality, I won't worry about
				// keeping track of the number of days played.
				RTC[3] = 0x00; // Days low 0x00 - 0xFF;
				RTC[4] = HaltTimer << 6;
				
				StartTime = std::chrono::system_clock::now();
			}
		}
		
		RisingEdge = 0x01;
		
	}
	else if (addr >= 0xA000 && addr < 0xC000)	// RTC / RAM
	{
		if (MappingRAM)
		{
			RAM[(RAMBankCode * 0x2000) + (addr % 0xA000)] = data;
		}
		else {
			StartTime = std::chrono::system_clock::now();

			RTC[RTCSelect] = data;

			// Ensure values written are within the correct ranges
			switch (RTCSelect)
			{
			case 0:
			case 1:		// Seconds or minutes
				if (data > 0x3B)
				{
					RTC[RTCSelect] = 0x3B;
				}
				break;
			case 2:		// Hours
				if (data > 0x17)
				{
					RTC[RTCSelect] = 0x17;
				}
				break;
			case 4:		// Day high
				// We will ignores the highest bit of day and 
				// the correspond carry

				// If the timer was halted and is no longer, then
				// update the StartTime variable.
				if (HaltTimer)
				{
					HaltTimer = (data >> 6) == 1;
					if (HaltTimer == 0)
					{
						StartTime = std::chrono::system_clock::now();
					}
				}
				else
				{
					HaltTimer = (data >> 6) == 1;
				}

				RTC[RTCSelect] = data & 0x40;

				break;
			}

		}
	}
}

uint8_t MBC3::read(uint16_t addr)
{
	if (addr >= 0x0000 && addr < 0x4000)	// Lower 16kiB ROM bank
	{
		return ROM[addr];
	}
	else if (addr >= 0x4000 && addr < 0x8000) // Upper 16kiB ROM bank
	{
		// Here the upper bank select is always used and we can switch 
		// freely between banks.
		return ROM[ROMBankCode * 0x4000 + (addr % 0x4000)];
	}
	else if (addr >= 0xA000 && addr < 0xC000) // RAM/RTC read
	{
		if (RAMEnable)
		{
			if (MappingRAM)
			{
				if (nRAMBanks != 0)
				{
					// Access any of up to 4 8kiB RAM banks
					return RAM[(RAMBankCode * 0x2000) + (addr % 0xA000)];
				}
			}
			else
			{
				return RTC[RTCSelect];
			}
		}
		else
		{
			// Open bus behaviour
			return 0xFF;
		}
	}


	return 0x00;
}