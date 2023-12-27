// gbEmu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Bus.hpp"

#include <cstdint>

int main()
{


    
    Bus bus("C:/Users/61481/Desktop/mb.gb");
    for (int i = 0; i < 1000000; i++)
    {
        bus.clock();
    }
}
