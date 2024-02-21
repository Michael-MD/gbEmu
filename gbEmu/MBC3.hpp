#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include "MBC.hpp"

using namespace std::chrono;

class MBC3 : public MBC
{
public:
	MBC3(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize);

	virtual void write(uint16_t addr, uint8_t data) override;
	virtual uint8_t read(uint16_t addr) override;

	// Registers
	uint8_t ROMBankCode, RAMBankCode;
	uint8_t RTC[5] = { 0 };
	bool RAMEnable;

	// Used for masking address to ROM size
	uint32_t ROMMask;

	// Used to keep track of whether RAM or RTC is mapped 
	// to 0xA000 - 0xC000
	bool MappingRAM;

	// Keeps track of rising edge
	uint8_t RisingEdge;

	// Keeps track of the selected RTC register
	uint8_t RTCSelect;

	// Stores unix timestamp since last reset or halt
	std::chrono::time_point<system_clock> StartTime;

	// Timer Halt
	bool HaltTimer;
};

