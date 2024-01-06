#pragma once
#include <cstdint>

class GB;

class Display
{
public:
	GB* gb;

	void connectGB(GB* gb);
	void clock();

	// ================== LCD Display Registers ==================
	const int GridWidth = 20 * 8;
	const int GridHeight = 18 * 8;
	uint8_t DotMatrix[18 * 8][20 * 8][4];		// ABGR

	// Line of Data being copied to LCD Driver
	uint8_t* LY;

	// LCD Control Register
	union LCDCRegister
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

	} *LCDC;

	// STAT
	union STATRegister
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

	} *STAT;

	uint8_t* SCY;	// Scroll Y
	uint8_t* SCX;	// Scroll X

	// Background Pixel Colour/Background Pallette Register
	uint8_t* BGP;
};