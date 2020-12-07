
#include "DSPWriter.h"
#include <Wire.h>
#include "DataConversion.h"

DSPWriter::DSPWriter() {}
DSPWriter::~DSPWriter() {}

void DSPWriter::downloadProgram()
{
    /*
    DSPWriter::writeRegisterBlock(REG_COREREGISTER_IC_1_ADDR, REG_COREREGISTER_IC_1_BYTE, R0_COREREGISTER_IC_1_Default, CORE_REGISTER_R0_REGSIZE);
    DSPWriter::writeRegisterBlock(PROGRAM_ADDR_IC_1, PROGRAM_SIZE_IC_1, Program_Data_IC_1, PROGRAM_REGSIZE);
    DSPWriter::writeRegisterBlock(PARAM_ADDR_IC_1, PARAM_SIZE_IC_1, Param_Data_IC_1, PARAMETER_REGSIZE);
    DSPWriter::writeRegisterBlock(REG_COREREGISTER_IC_1_ADDR, R3_HWCONFIGURATION_IC_1_SIZE, R3_HWCONFIGURATION_IC_1_Default, HARDWARE_CONF_REGSIZE);
    DSPWriter::writeRegisterBlock(REG_COREREGISTER_IC_1_ADDR, REG_COREREGISTER_IC_1_BYTE, R4_COREREGISTER_IC_1_Default, CORE_REGISTER_R4_REGSIZE);*/
}

void DSPWriter::writeRegisterBlock(uint16_t subAddress, int dataLength, const uint8_t* pdata, uint8_t registerSize)
{
    uint8_t MSByte = 0;
    uint8_t LSByte = 0;
    uint16_t bytesSent = 0;

    // Run until all bytes are sent
    while (bytesSent < dataLength)
    {
        // Convert address to 8-bit
        MSByte = subAddress >> 8;
        LSByte = (uint8_t)(subAddress & 0xFF);

        Wire.beginTransmission(DSP_I2C_ADDRESS);
        Wire.write(MSByte); // Send high address
        Wire.write(LSByte); // Send low address
        for (uint8_t i = 0; i < registerSize; i++) // Send n bytes
        {
            Wire.write(pdata[bytesSent]);
            bytesSent++;
        }
        Wire.endTransmission();

        subAddress++; // Increase address
    }
}

void DSPWriter::writeRegister(uint16_t memoryAddress, uint8_t length, const uint8_t* data)
{
    uint8_t LSByte = (uint8_t)memoryAddress & 0xFF;
    uint8_t MSByte = memoryAddress >> 8;

    Wire.beginTransmission(DSP_I2C_ADDRESS); // Begin write

    Wire.write(MSByte); // Send high address
    Wire.write(LSByte); // Send low address

    for (uint8_t i = 0; i < length; i++)
        Wire.write(data[i]); // Send all bytes in passed array

    Wire.endTransmission(); // Write out data to I2C and stop transmitting
}


void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, uint8_t* data, bool finished)
{
    static uint8_t _safeload_count = 0; // Keeps track of the safeload count

    uint8_t addr[2]; // Address array

    addr[0] = (memoryAddress >> 8) & 0xFF;
    addr[1] = memoryAddress & 0xFF;

    DSPWriter::writeRegister(dspRegister::SafeloadAddress0 + _safeload_count, sizeof(addr), addr); // Place passed 16-bit memory address in safeload address area

    // Q: Why is the safeload registers five bytes long, while I'm loading four-byte parameters into the RAM using these registers?
    // A: The safeload registers are also used to load the slew RAM data, which is five bytes long. For parameter RAM writes using safeload,
    // the first byte of the safeload register can be set to 0x00.

    // Needs 5 bytes of data
    DSPWriter::writeRegister(dspRegister::SafeloadData0 + _safeload_count, 5, data); // Placed passed data (5 bytes) in the next safeload data space

    _safeload_count++; // Increase counter

    if (finished == true || _safeload_count >= 5) // Max 5 safeload memory registers
    {
        addr[0] = 0x00;
        addr[1] = 0x3C; // Set the IST bit (initiate safeload transfer bit)
        DSPWriter::writeRegister(dspRegister::CoreRegister, sizeof(addr), addr); // Load content from the safeload registers
        _safeload_count = 0;
    }
}

void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, int32_t data, bool finished)
{
    uint8_t dataArray[5];
    DataConversion::intToFixed(data, dataArray);
    safeload_writeRegister(memoryAddress, dataArray, finished);
}

void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, float data, bool finished)
{
    uint8_t dataArray[5];
    DataConversion::floatToFixed(data, dataArray);
    safeload_writeRegister(memoryAddress, dataArray, finished);
}

void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, int16_t data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, uint32_t data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, uint16_t data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, uint8_t data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
void DSPWriter::safeload_writeRegister(uint16_t memoryAddress, double data, bool finished) { safeload_writeRegister(memoryAddress, (float)data, finished); }