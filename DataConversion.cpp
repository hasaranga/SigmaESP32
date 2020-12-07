
#include "DataConversion.h"

void DataConversion::floatToFixed(float value, uint8_t* buffer)
{
    // Convert float to 4 byte hex
    int32_t fixedval = (value * ((int32_t)1 << 23));

    // Store the 4 bytes in the passed buffer
    buffer[0] = 0x00; // First must be empty
    buffer[1] = (fixedval >> 24) & 0xFF;
    buffer[2] = (fixedval >> 16) & 0xFF;
    buffer[3] = (fixedval >> 8) & 0xFF;
    buffer[4] = fixedval & 0xFF;
}

/***************************************
Function: intToFixed()
Purpose:  Converts a 28.0 integer value to 5-byte HEX and stores it to a buffer
Inputs:   int32_t value;      Value to convert
          uint8_t *buffer;    Buffer to store the converted data to
Returns:  None
***************************************/
void DataConversion::intToFixed(int32_t value, uint8_t* buffer)
{
    // Store the 4 bytes in the passed buffer
    buffer[0] = 0x00; // First must be empty
    buffer[1] = (value >> 24) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 8) & 0xFF;
    buffer[4] = value & 0xFF;
}

/***************************************
Function: floatToInt()
Purpose:  Converts a 5.23 float value to int 28.0
Inputs:   float value;    Value to convert
Returns:  int32_t;        Converted value
***************************************/
int32_t DataConversion::floatToInt(float value)
{
    // Convert float 5.23 to int 28.0
    return (value * ((int32_t)1 << 23));
}