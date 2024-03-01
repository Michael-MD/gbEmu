#include "DMA.hpp"
#include "GBInternal.hpp"

void DMA::connectGB(GBInternal* gb)
{
	this->gb = gb;

	DMAReg = gb->RAM + 0xFF46;
}

void DMA::clock()
{
	// Check if DMA is active
	if (DMAinProgress)
	{
		// Check if this is the first cycle.
		// If so then set a count down for how
		// long the transfer should take in T-cycles.
		if (TransferTicks == 0xFFFF)
		{
			TransferTicks = 640;

			// Although the transfer happens gradually,
			// we will have it happen all at once and 
			// nothing will be done in the remaining cycles.
			uint16_t StartAddr = *DMAReg << 8;

			for (uint16_t i = 0; i < 0x100; i++) 
			{
				gb->RAM[0xFE00 + i] = gb->RAM[StartAddr + i];
			}
		}

		// End of transfer
		if (TransferTicks == 0)
		{
			TransferTicks = 0xFFFF;
			DMAinProgress = false;
		}
		else
		{
			TransferTicks--;
		}
	}

}