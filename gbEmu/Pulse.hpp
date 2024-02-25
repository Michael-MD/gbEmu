#pragma once
#include "Divider.hpp"
#include "SoundChannel.hpp"

#include <cstdint>

class Pulse : public SoundChannel
{
public:
	Pulse();
	~Pulse();

	virtual int8_t GetSample() override;
	void clock();

	// Dividers
	Divider<uint16_t>* PeriodDiv;

	// Keeps track of the period which is 
	// broken up between registers NRx3 and NRx4.
	uint16_t PeriodValue;

private:

};

