#pragma once
#include <cstdint>

class GBInternal;


/// <summary>
/// The DMA(Direct memory access) unit allows for 
/// data to be transfered quickly (quicker than the cpu)
/// from some space in memory to the OAM.
/// There are some nuances which I haven't implemented here 
/// since the behaviour will only arise if the programmer has
/// made a mistake.
/// </summary>
class DMA
{
public:
	GBInternal* gb;

	void connectGB(GBInternal* gb);
	void clock();

	uint8_t* DMAReg;
	bool DMAinProgress = false;

	uint16_t TransferTicks = 0xFFFF;

private:

};