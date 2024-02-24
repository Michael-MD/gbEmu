#pragma once
#include <cstdint>
#include "Divider.hpp"

class GB;

class SoundChannel
{
public:
	GB* gb;

	void connectGB(GB* gb);
	virtual int8_t GetSample() = 0;

	// Default volume will be mid-range at 7.
	// Volume should always remain within the range
	// 0 to 15 (inclusive).
	uint8_t Volume = 7;

	// Keeps track of whether the channel
	// is musted or not for whatever reason.
	bool Mute = false;
};

