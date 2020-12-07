
#pragma once

#include <stdint.h>

class DataConversion
{
public:

    /***************************************
    Function: floatTofixed()
    Purpose:  Converts a 5.23 float value to 5-byte HEX and stores it to a buffer
    Inputs:   float value;      Value to convert
              uint8_t *buffer;  Buffer to store the converted data to
    Returns:  None
    ***************************************/
    static void floatToFixed(float value, uint8_t* buffer);

    /***************************************
    Function: intToFixed()
    Purpose:  Converts a 28.0 integer value to 5-byte HEX and stores it to a buffer
    Inputs:   int32_t value;      Value to convert
              uint8_t *buffer;    Buffer to store the converted data to
    Returns:  None
    ***************************************/
    static void intToFixed(int32_t value, uint8_t* buffer);

    /***************************************
    Function: floatToInt()
    Purpose:  Converts a 5.23 float value to int 28.0
    Inputs:   float value;    Value to convert
    Returns:  int32_t;        Converted value
    ***************************************/
    static int32_t floatToInt(float value);

};