// gbEmu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "GB.hpp"

#include <cstdint>

int main()
{


    
    GB gb("C:/Users/61481/Desktop/mb.gb");
    for (int i = 0; i < 1000000; i++)
    {
        gb.clock();
    }
}
