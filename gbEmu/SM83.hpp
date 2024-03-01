#pragma once

#include <cstdint>
#include <string>
#include <functional>

class GBInternal;

class SM83
{
	friend GBInternal;

public:
	SM83();
	void reset();

	void connectGB(GBInternal *gb);

	void clock();	// Clocks SM83

	GBInternal* gb;

	uint32_t nMachineCycles = 0;

private:

	// ============== Registers ============== 
	
	// General Purpose Registers (GPR)

	// Auxillary Registers
	union
	{
		struct
		{
			union
			{
				struct
				{
					uint8_t U : 4;	// Unused
					uint8_t CY : 1;	// Carry
					uint8_t HC : 1;	// Half Carry
					uint8_t N : 1;	// Subtract
					uint8_t Z : 1;	// Zero
				};

				uint8_t F;
			};
			
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
	inline std::string qqString(uint8_t i); // For Disassembly

	inline uint16_t& ss(uint8_t i);

	uint16_t PC, SP; // Program Counter, Stack Pointer

	// ============== Instructions ==============
	uint8_t a, b;

	struct
	{
		std::function<std::string()> mnemonic;
		std::function<void()> op;
		uint8_t cycles;
		uint8_t a;
		uint8_t b;
	} InstructionSet[256], 
		CurrentInstruction, 
		InstructionSet16Bit[256],
		CurrentInstruction16Bit;

	uint8_t cycle;		// Cycle Number

	uint8_t IMEDelaySet = 0xFF;

	// Interrupt Handling

	bool IME; // Interrupt Master Flag

	void InterruptServiceRoutine();

	bool Halted, Stopped; // Set if CPU is halted/stopped

	// The halt instruction exhibits specific behaviour
	// when executed while an interrupt is pending.
	// This flag checks this.
	bool PendingInterruptWhileHalted;

	// The halt bug causes the following instruciton to
	// be executed twice due to a failure in incrementing
	// the PC counter.
	bool RereadInstruction;

};