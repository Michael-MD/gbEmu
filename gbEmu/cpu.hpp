#pragma once

#include <cstdint>

class CPU
{
private:

	// ============== Registers ============== 
	
	// General Purpose Registers (GPR)
	union
	{
		struct
		{
			uint8_t F : 8;
			uint8_t A : 8;
			uint8_t C : 8;
			uint8_t B : 8;
			uint8_t E : 8;
			uint8_t D : 8;
			uint8_t L : 8;
			uint8_t H : 8;
		};

		struct
		{
			uint16_t AF : 16;
			uint16_t BC : 16;
			uint16_t DE : 16;
			uint16_t HL : 16;
		};
	};

	uint16_t PC, SP; // Program Counter, Stack Pointer

	// Flags
	union
	{
		struct
		{
			uint8_t _ : 4;
			uint8_t C : 1;	// Carry
			uint8_t H : 1;	// Half Carry
			uint8_t N : 1;	// Subtract
			uint8_t Z : 1;	// Zero
		};

		uint8_t FlagReg;
	};

	// ============== Instructions ==============



};