#pragma once

#include <cstdint>
#include <string>
#include <functional>

class GB;

class SM83
{
	friend GB;

public:
	SM83();
	void reset();

	void connectGB(GB *gb);

	void clock();	// Clocks SM83

	GB* gb;

	int nMachineCycles = 0;

private:
	
	// DMG mode has only 8kB of built-in ppulay RAM
	uint8_t PPURAM[8 * 1024];	// LCD PPU RAM

	// ============== Registers ============== 
	
	// General Purpose Registers (GPR)

	// Auxillary Registers
	union
	{
		struct
		{
			uint8_t F;
			uint8_t A;
			uint8_t C;
			uint8_t B;
			uint8_t E;
			uint8_t D;
			uint8_t L;
			uint8_t H;
		};

		struct
		{
			uint16_t AF;
			uint16_t BC;
			uint16_t DE;
			uint16_t HL;
		};
	};

	inline uint8_t& GPR(uint8_t i); 
	inline char GPRString(uint8_t i); // For Disassembly
	
	inline uint16_t& qq(uint8_t i);
	inline uint16_t& ss(uint8_t i);

	uint16_t PC, SP; // Program Counter, Stack Pointer

	// Flags
	union
	{
		struct
		{
			uint8_t _ : 4;
			uint8_t CY : 1;	// Carry
			uint8_t HC : 1;	// Half Carry
			uint8_t N : 1;	// Subtract
			uint8_t Z : 1;	// Zero
		};

		uint8_t FlagReg;
	};

	// ============== Instructions ==============
	uint8_t a, b;

	struct
	{
		std::function<std::string()> mnemonic;
		std::function<void()> op;
		uint8_t cycles;
		uint8_t a;
		uint8_t b;
	} InstructionSet[256], CurrentInstruction;

	union
	{
		struct
		{
			uint8_t op0 : 3;
			uint8_t op1 : 3;
			uint8_t op2 : 2;
		};

		uint8_t Opcode;
	};

	uint8_t cycle;		// Cycle Number


};