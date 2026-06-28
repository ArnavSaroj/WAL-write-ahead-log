#include "../include/crc32.h"
#include <cstddef>
#include <cstdint>
#include<iostream>

using namespace std;

uint32_t calculate_crc(const uint8_t *data, size_t length)
{

    const uint32_t POLYNOMIAL = 0xEDB88320;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t byte = 0; byte < length; byte++)
    {
        crc = crc ^ data[byte];
        for (uint32_t bit = 0; bit < 8; bit++)
        {
            if (crc & 1)
            {
                crc = crc >> 1;
                crc = crc ^ POLYNOMIAL;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    return ~crc;
}




