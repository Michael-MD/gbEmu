#include "PPU.hpp"
#include "GB.hpp"

void PPU::connectGB(GB* gb)
{
	this->gb = gb;

	// Reference PPU Registers to Main Registers Block
	LY = gb->RAM + 0xFF44;
	LCDC = reinterpret_cast<LCDCRegister*>(gb->RAM + 0xFF40);
	STAT = reinterpret_cast<STATRegister*>(gb->RAM + 0xFF41);
	SCY = gb->RAM + 0xFF42;
	SCX = gb->RAM + 0xFF43;
	BGP = gb->RAM + 0xFF47;
	
	// Set-up pixel rendering
	Mode = OAMScan;
	DotsRemaining = 0;
	DotsTotal = 0;
	CurrentPixelRendered = 0;
}

void PPU::clock()
{
	// ============= LCD PPU ============= 
	DotsTotal++;

	if (DotsRemaining == 0)
	{
		if ((*LY) >= 144)
		{
			Mode = VerticalBlank;
			DotsRemaining = 456;
			DotsTotal = 0;					// Reset Row Dots Count
			gb->IF->VerticalBlanking = 1;
			(*LY) = ((*LY) + 1) % 154;		// Next Row
		}

		switch (Mode)
		{
		case OAMScan:
			Mode = DrawingPixels;
			DotsRemaining = 172;
			CurrentPixelRendered = 0;
			break;
		case DrawingPixels:
			Mode = HorizontalBlank;
			DotsRemaining = 376 - DotsTotal;
			break;
		case HorizontalBlank:
			Mode = OAMScan;
			DotsRemaining = 80;
			DotsTotal = 0;					// Reset Row Dots Count
			gb->IF->VerticalBlanking = 0;	// Reset Vertical Blanking Flag
			(*LY)++;		// Next Row will now begin to be rendered
			break;
		}
	}
	else
	{
		DotsRemaining--;
	}

	// =========== Draw Pixels ===========
	if (Mode == DrawingPixels)
	{
		if (LCDC->bBG == 1 && CurrentPixelRendered < 160) // Check if background rendering enabled
		{
			uint8_t CHRCodesStartAddr = LCDC->BGCodeArea ? 0x9C00 : 0x9800;
			uint8_t BGAddrModeStartAddr = LCDC->BGCharData ? 0x8000 : 0x9000;

			uint8_t CHRCode = gb->RAM[CHRCodesStartAddr + (int)((*LY) / 8) * 32 + (int)(CurrentPixelRendered / 8)];

			// Find Corresponding Tile
			uint8_t DotDataAddr;
			if (LCDC->BGCharData == 0)
			{
				DotDataAddr = BGAddrModeStartAddr + (int8_t)CHRCode * 0x0F;
			}
			else
			{
				DotDataAddr = BGAddrModeStartAddr + CHRCode * 0x0F;
			}

			// Parse Dot Data
			uint8_t TileLO = gb->RAM[DotDataAddr + ((*LY) & 8) * 2 + 0];
			uint8_t TileHI = gb->RAM[DotDataAddr + ((*LY) & 8) * 2 + 1];

			// Get Pixel Shade
			int p = (*LY) & 8;
			uint8_t PixelPalette = (((TileHI >> p) & 0x01) << 1) | ((TileLO >> p) & 0x01);

			// Store Result PPU Grid
			uint8_t value = (((*BGP) >> (PixelPalette * 2)) & 0b11) * 255;

			DotMatrix[*LY][CurrentPixelRendered][0] = 255;
			DotMatrix[*LY][CurrentPixelRendered][1] = value;
			DotMatrix[*LY][CurrentPixelRendered][2] = value;
			DotMatrix[*LY][CurrentPixelRendered][3] = value;

			CurrentPixelRendered++;
		}
	}

	
}