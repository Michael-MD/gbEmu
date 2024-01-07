#pragma once
#include <cstdint>
#include "SM83.hpp"
#include "Cartridge.hpp"
#include "PPU.hpp"
#include <string>
#include "SDL.h"


class GB
{
public:
	GB(std::string gbFilename);

	SM83 cpu;
	Cartridge* cart;
	PPU ppu;

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

	// NR10 Register
	uint8_t* NR10 = RAM + 0xFF10;

	// NR11 Register
	uint8_t* NR11 = RAM + 0xFF11;

	// NR12 Register
	uint8_t* NR12 = RAM + 0xFF12;

	// NR14 Register
	uint8_t* NR14 = RAM + 0xFF14;

	// NR21 Register
	uint8_t* NR21 = RAM + 0xFF16;

	// NR22 Register
	uint8_t* NR22 = RAM + 0xFF17;

	// NR24 Register
	uint8_t* NR24 = RAM + 0xFF19;

	// NR30 Register
	uint8_t* NR30 = RAM + 0xFF1A;

	// NR31 Register
	uint8_t* NR31 = RAM + 0xFF1B;

	// NR32 Register
	uint8_t* NR32 = RAM + 0xFF1C;

	// NR33 Register
	uint8_t* NR33 = RAM + 0xFF1E;

	// NR41 Register
	uint8_t* NR41 = RAM + 0xFF20;

	// NR42 Register
	uint8_t* NR42 = RAM + 0xFF21;

	// NR43 Register
	uint8_t* NR43 = RAM + 0xFF22;

	// NR44 Register
	uint8_t* NR44 = RAM + 0xFF23;

	// NR50 Register
	uint8_t* NR50 = RAM + 0xFF24;

	// NR51 Register
	uint8_t* NR51 = RAM + 0xFF25;

	// NR52 Register
	uint8_t* NR52 = RAM + 0xFF26;


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

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
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

	bool IME; // Interrupt Master Flag

private:	
	void handleEvents();
	void update();
	void render();
	void clean();

	bool bIsRunning = true;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	const float GB_SCREEN_RATIO = 20 / 18;	// height / width
};