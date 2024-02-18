#pragma once
#include <cstdint>

class GB;


/// <summary>
/// The DMA(Direct memory access) unit allows for 
/// data to be transfered quickly (quicker than the cpu)
/// to from some space in memory to the OAM.
/// There are some nuances which I haven't implemented here 
/// since the behaviour will only arise if the programmer has
/// made a mistake.
/// </summary>
class DMA
{
public:
	GB* gb;

	void connectGB(GB* gb);
	void clock();

	uint8_t* DMAReg;
	bool DMAinProgress = false;

	uint16_t TransferTicks = 0xFFFF;

private:

};