// gbEmu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Bus.hpp"

#include <cstdint>

int main()
{


    
    Bus bus;
    for (int i = 0; i < 100; i++)
    {
        bus.clock();
    }
}
