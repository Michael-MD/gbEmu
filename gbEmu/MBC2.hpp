#pragma once
#include <cstdint>
#include <string>
#include "MBC.hpp"

class MBC2 : public MBC
{
public:
	MBC2(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize);

	virtual void write(uint16_t addr, uint8_t data) override;
	virtual uint8_t read(uint16_t addr) override;

	// Registers
	uint8_t ROMBankCode;
	bool RAMEnable;
};

