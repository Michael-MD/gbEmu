// gbEmu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "GB.hpp"

#include <cstdint>

#include "SDL.h"

int main(int argc, char* argv[])
{
    //GB gb("C:/Users/61481/Desktop/tetris.gb");
    GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/01-special.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/02-interrupts.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/03-op sp,hl.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/04-op r,imm.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/05-op rp.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/06-ld r,r.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/08-misc instrs.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/09-op r,r.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/10-bit ops.gb");
    //GB gb("C:/Users/61481/Documents/code/gb/gb-test-roms-master/cpu_instrs/individual/11-op a,(hl).gb");
    //GB gb("C:/Users/61481/Documents/code/gb/mts-20240108-1545-b5740e1/acceptance/instr/daa.gb");
    /*for (uint8_t i = 0; i < 1000000; i++)
    {
        gb.clock();
    }*/

    return 0;
}
