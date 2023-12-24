#pragma once

#include <cstdint>
#include <string>
#include <functional>

class Bus;

class CPU
{
public:
	CPU();

	void clock();	// Clocks CPU

private:
	Bus *bus;

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
			uint16_t AF : 16;
			uint16_t BC : 16;
			uint16_t DE : 16;
			uint16_t HL : 16;
		};
	};

	inline uint8_t& GPR(uint8_t i);

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

	struct
	{
		std::string mnemonic;
		std::function<void()> op;
		uint8_t cycles;

	} InstructionSet[256];

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

	uint8_t cycle;		// Cycle number


};