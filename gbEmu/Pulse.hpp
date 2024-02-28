#pragma once
#include "SoundChannel.hpp"

#include <cstdint>

class Pulse : public SoundChannel
{
public:
	Pulse();
	~Pulse();

	virtual uint8_t GetSample() override;
	void clock();

private:

};

