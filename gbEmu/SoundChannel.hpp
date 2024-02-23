#pragma once
#include <cstdint>

class SoundChannel
{
public:
	virtual uint8_t GetSample() = 0;
};

