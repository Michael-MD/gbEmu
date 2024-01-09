#pragma once
#include <cstdint>
#include <string>
#include "MBC.hpp"

class MBC1 : public MBC
{
public:
	MBC1(std::string gbFilename, uint8_t ROMSize, uint8_t RAMSize);

	virtual void write(uint16_t addr, uint8_t data) override;
	virtual uint8_t read(uint16_t addr) override;

	// Registers
	uint8_t ROMBankCode, UpperROMBankCode, bBankingMode;
	bool bRAMEnable;
};