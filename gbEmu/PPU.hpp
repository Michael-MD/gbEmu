#pragma once
#include <cstdint>

class GB;

class PPU
{
public:
	GB* gb;

	void connectGB(GB* gb);
	void clock();
	void reset();

	int DotsRemaining;
	int DotsTotal;

	enum
	{
		OAMScan = 2,
		DrawingPixels = 3,
		HorizontalBlank = 0,
		VerticalBlank = 1,
	} Mode;

	// ================== LCD PPU Registers ==================
	const int GridWidth = 20 * 8;
	const int GridHeight = 18 * 8;
	uint8_t DotMatrix[18 * 8][20 * 8][4];		// ABGR

	// Line of data being copied to LCD Driver
	uint8_t* LY;

	// LY register compare
	uint8_t* LYC;

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

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};

	} *LCDC;

	bool bLineRendered = false;

	// STAT
	union STATRegister
	{
		struct
		{
			union
			{
				struct 
				{
					uint8_t ModeFlag : 2;
					uint8_t MatchFlag : 1;
					uint8_t Mode00Selection : 1;
					uint8_t Mode01Selection : 1;
					uint8_t Mode10Selection : 1;
					uint8_t LYCMatch : 1;
				};

				struct
				{
					uint8_t OtherFlags : 3;
					uint8_t InterruptSelection : 4;
				};
			};
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
			reg |= 0x80;	// Highest bit is always set
		};

	} *STAT;

	uint8_t* SCY;	// Scroll Y
	uint8_t* SCX;	// Scroll X

	// Background Pixel Colour/Background Pallette Register
	uint8_t* BGP;

	// LY setter for handling various instantanious changes and wrap around requried when LY is changed
	void setLY(uint8_t v);

private:
	int LX;
};