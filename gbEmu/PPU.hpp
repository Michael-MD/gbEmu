#pragma once
#include <cstdint>

class GBInternal;

class PPU
{
public:
	GBInternal* gb;

	void connectGB(GBInternal* gb);
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
			uint8_t OBJ8x16 : 1;
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

	uint8_t* WX;	// Window position Y
	uint8_t* WY;	// Window position X

	// Background Pixel Colour/Background Pallette Register
	uint8_t* BGP;

	// Sprites pallettes
	uint8_t* OBP0;
	uint8_t* OBP1;

	// LY setter for handling various instantanious changes and wrap around requried when LY is changed
	void setLY(uint8_t v);

private:
	int LX;

	// Keeps track of delays during mode 3
	uint32_t Delay = 0;

	// Keeps track of window scanline to be rendered
	uint8_t WLY = 0;

	// Useful data strucutre for interpreting OAM objects
	struct Object
	{
		uint8_t YPos;
		uint8_t XPos;
		uint8_t TileIndex;
		union
		{
			struct
			{
				uint8_t CGBOnly : 4; // Used on in CGB
				uint8_t Pallette : 1;
				//	0 : OBP0
				//	1 : OBP1
				uint8_t XFlip : 1;	// 1 : Entire OBJ is horizontally mirrored
				uint8_t YFlip : 1;	// 1 : Entire OBJ is vertically mirrored
				uint8_t Priority : 1;
				//	1 : BG and Window colors 1–3 are drawn over this OBJ
				//	0 : Obj is drawn infront
			};

			uint8_t Attributes; // Attrubtes/flags
		};
	} *ScanLineObjects[10];


	bool FoundObject = false;

	// Flags when the found objects prioirty may
	// cause it to appear behind the background
	// and window.
	bool ObjectPriorityConflict = false;
	
	uint8_t nScanLineObjects = 0;
	
};