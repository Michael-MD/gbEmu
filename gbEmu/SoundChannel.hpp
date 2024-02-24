#pragma once
#include <cstdint>
#include "Divider.hpp"

class GB;

class SoundChannel
{
public:
	GB* gb;

	void connectGB(GB* gb);
	virtual int16_t GetSample() = 0;

	// Keeps track of whether the channel
	// is musted or not for whatever reason.
	bool Mute = false;
};

