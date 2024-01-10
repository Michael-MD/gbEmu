// gbEmu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "GB.hpp"

#include <cstdint>

#include "SDL.h"

int main(int argc, char* argv[])
{
    //GB gb("C:/Users/61481/Desktop/tetris.gb");
    GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/10-bit ops.gb");
    /*for (uint8_t i = 0; i < 1000000; i++)
    {
        gb.clock();
    }*/

    return 0;
}
