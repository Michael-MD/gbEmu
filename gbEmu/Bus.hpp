#pragma once
#include <cstdint>
#include "CPU.hpp"

class Bus
{
public:
	CPU *cpu;

	int nClockCycles;

	uint8_t read(uint16_t addr);
	void write(uint16_t addr, uint8_t data);
	void clock();
	

	// RAM for Memory Mapping
	uint8_t RAM[0xFFFF + 1];

	// ========= Memory Mapped Registers ========= 

	// Controller Data
	union
	{
		struct
		{
			uint8_t P10 : 1;
			uint8_t P11 : 1;
			uint8_t P12 : 1;
			uint8_t P13 : 1;
			uint8_t P14 : 1;
			uint8_t P15 : 1;
		};

		uint8_t reg_;

		void operator=(uint8_t reg)
		{
			reg_ = reg;
		};

	} *P1 = reinterpret_cast<decltype(P1)>(RAM + 0xFF00);

	// Divider (Read/Reset)
	/*union
	{
		struct
		{
			uint8_t f9 : 1;
			uint8_t f10 : 1;
			uint8_t f11 : 1;
			uint8_t f12 : 1;
			uint8_t f13 : 1;
			uint8_t f14 : 1;
			uint8_t f15 : 1;
			uint8_t f16 : 1;
		};

		uint8_t reg_;

		void operator=(uint8_t reg)
		{
			reg_ = reg;
		};

	} *Div = reinterpret_cast<decltype(Div)>(RAM + 0xFF04);*/
	uint8_t* Div = RAM + 0xFF04;

	// TIMA Register
	uint8_t* TIMA = RAM + 0xFF05;

	// TMA Register
	uint8_t* TMA = RAM + 0xFF06;

	// TAC Register
	union
	{
		struct
		{
			uint8_t InputClockSelect : 2;
			uint8_t Start : 1;
		};

		uint8_t reg_;

		void operator=(uint8_t reg)
		{
			reg_ = reg;
		};

	} *TAC = reinterpret_cast<decltype(TAC)>(RAM + 0xFF07);

	// ================== Interrupt Registers ==================

	// Interrupt Request
	union
	{
		struct
		{
			uint8_t VerticalBlanking : 1;
			uint8_t LCDC : 1;
			uint8_t TimerOverflow : 1;
			uint8_t SerialIOTransferCompletion : 1;
			uint8_t PNegEdge : 1;
		};

		uint8_t reg_;

		void operator=(uint8_t reg)
		{
			reg_ = reg;
		};

	} *IF = reinterpret_cast<decltype(IF)>(RAM + 0xFF0F);

	// Interrupt Enable
	union
	{
		struct
		{
			uint8_t VerticalBlanking : 1;
			uint8_t LCDC : 1;
			uint8_t TimerOverflow : 1;
			uint8_t SerialIOTransferCompletion : 1;
			uint8_t PNegEdge : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};

	} *IE = reinterpret_cast<decltype(IE)>(RAM + 0xFFFF);

	bool IME; // Interrupt Mater Flag

	// ================== LCD Display Registers ==================

	// LCD Control Register
	union
	{
		struct
		{
			
			uint8_t bBG : 1;
			uint8_t bOBJ : 1;
			uint8_t OBJBlockComposition : 1;
			uint8_t BGCodeArea : 1;
			uint8_t BGCharData : 1;
			uint8_t bWindowing : 1;
			uint8_t WindowCodeArea : 1;
			uint8_t bLCDC : 1;
		};

		uint8_t reg_;

		void operator=(uint8_t reg)
		{
			reg_ = reg;
		};

	} *LCDC = reinterpret_cast<decltype(LCDC)>(RAM + 0xFF40);

	// STAT
	union
	{
		struct
		{

			uint8_t ModeFlag : 2;
			uint8_t MatchFlag : 1;
			uint8_t InterruptSelection : 3;

		};

		uint8_t reg_;

		void operator=(uint8_t reg)
		{
			reg_ = reg;
		};

	} *STAT = reinterpret_cast<decltype(STAT)>(RAM + 0xFF41);

	uint8_t* SCY = RAM + 0xFF42;	// Scroll Y
	uint8_t* SCX = RAM + 0xFF43;	// Scroll X


};