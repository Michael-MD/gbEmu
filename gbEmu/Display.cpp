#include "Display.hpp"
#include "GB.hpp"

void Display::connectGB(GB* gb)
{
	this->gb = gb;

	// Reference Display Registers to Main Registers Block
	LY = gb->RAM + 0xFF44;
	LCDC = reinterpret_cast<LCDCRegister*>(gb->RAM + 0xFF40);
	STAT = reinterpret_cast<STATRegister*>(gb->RAM + 0xFF41);
	SCY = gb->RAM + 0xFF42;
	SCX = gb->RAM + 0xFF43;
	BGP = gb->RAM + 0xFF47;

}

void Display::clock()
{
	// ============= LCD Display ============= 
	if (gb->nClockCycles % 1890 == 0) // Do Entire Row at Once
	//if (gb->nClockCycles % 8 == 0) // Do Entire Row at Once
	{
		if(LCDC->bBG == 1) // Check if background rendering enabled
		{
			// TODO: SCX
			// TODO: Fix indexing into appropriate row

			uint8_t CHRCodesStartAddr = LCDC->BGCodeArea ? 0x9C00 : 0x9800;
			uint8_t BGAddrModeStartAddr = LCDC->BGCharData ? 0x8000 : 0x9000;

			// If not in vertical blanking period get pixel data
			if (gb->IF->VerticalBlanking == 0)
			{
				for (int ColBlock = 0; ColBlock < 20; ColBlock++)
				{
					// Get CHR Code
					uint8_t CHRCode = gb->RAM[CHRCodesStartAddr + (int)((*LY) / 8) * 32 + ColBlock];

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

					// Get pixel Shade for entire row of pixels
					for (int p = 0; p < 8; p++)
					{
						uint8_t PixelPalette = (((TileHI >> p) & 0x01) << 1) | ((TileLO >> p) & 0x01);

						// Store Result Display Grid
						uint8_t value = (((*BGP) >> (PixelPalette * 2)) & 0b11) * 255;

						DotMatrix[*LY][ColBlock * 8 + p][0] = 255;
						DotMatrix[*LY][ColBlock * 8 + p][1] = value;
						DotMatrix[*LY][ColBlock * 8 + p][2] = value;
						DotMatrix[*LY][ColBlock * 8 + p][3] = value;
					}
				}
			}

			(*LY) = ((*LY) + 1) % 154;

			// Check if in Vertical Blanking Region
			if (*LY == 18 * 8 - 1)
			{
				gb->IF->VerticalBlanking = 1;
			}
			else if (*LY == 0)
			{
				gb->IF->VerticalBlanking = 0;
			}
		}

	}


	// TODO: window display
}