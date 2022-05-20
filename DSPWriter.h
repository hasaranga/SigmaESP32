
#pragma once

#include <stdint.h>

#define CORE_REGISTER_R0_REGSIZE 2
#define HARDWARE_CONF_REGSIZE 1
#define CORE_REGISTER_R4_REGSIZE 2
#define PARAMETER_REGSIZE 4
#define PROGRAM_REGSIZE 5

/* 7-bit i2c addresses */
#define DSP_I2C_ADDRESS (0x68 >> 1) & 0xFE
#define EEPROM_I2C_ADDRESS (0xa0 >> 1) & 0xFE

// Hardware register constants
typedef enum
{
    InterfaceRegister0 = 0x0800,
    InterfaceRegister1 = 0x0801,
    InterfaceRegister2 = 0x0802,
    InterfaceRegister3 = 0x0803,
    InterfaceRegister4 = 0x0804,
    InterfaceRegister5 = 0x0805,
    InterfaceRegister6 = 0x0806,
    InterfaceRegister7 = 0x0807,
    GpioAllRegister = 0x0808,
    Adc0 = 0x0809,
    Adc1 = 0x080A,
    Adc2 = 0x080B,
    Adc3 = 0x080C,
    SafeloadData0 = 0x0810,
    SafeloadData1 = 0x0811,
    SafeloadData2 = 0x0812,
    SafeloadData3 = 0x0813,
    SafeloadData4 = 0x0814,
    SafeloadAddress0 = 0x0815,
    SafeloadAddress1 = 0x0816,
    SafeloadAddress2 = 0x0817,
    SafeloadAddress3 = 0x0818,
    SafeloadAddress4 = 0x0819,
    DataCapture0 = 0x081A,
    DataCpature1 = 0x081B,
    CoreRegister = 0x081C,
    RAMRegister = 0x081D,
    SerialOutRegister1 = 0x081E,
    SerialInputRegister = 0x081F,
    MpCfg0 = 0x0820,
    MpCfg1 = 0x0821,
    AnalogPowerDownRegister = 0x0822,
    AnalogInterfaceRegister0 = 0x0824
} dspRegister;

class DSPWriter
{
public:
	DSPWriter();
	~DSPWriter();

    // DSP data write methods

    static void downloadProgram();

    /***************************************
    Function: writeRegisterBlock()
    Inputs:
      uint16_t dataLength;               Number of bytes to write
      const uint8_t *pdata;           Data array to write
      uint8_t registerSize;          Number of bytes each register can hold
    ***************************************/
    static void writeRegisterBlock(uint16_t subAddress, int dataLength, const uint8_t* pdata, uint8_t registerSize);

    /***************************************
    Function: writeRegister()
    Purpose:  Writes data to the DSP
              (max 32 bytes due to i2c buffer size)
    Inputs:   uint16_t startMemoryAddress;   DSP memory address
              uint8_t length;                Number of bytes to write
              uint8_t *data;                 Data array to write
    Returns:  None
    ***************************************/
    static void writeRegister(uint16_t memoryAddress, uint8_t length, const uint8_t* data);

    // Template wrapper for safeload_writeRegister
    // Handles both int 28.0 and float 5.23 formatted parameters
    // Usage: safeload_write(uint16_t startMemoryAddress, data1, data2, ..., dataN);
    //template <typename Address, typename Data1, typename... DataN>
    //void safeload_write(const Address &address, const Data1 &data1, const DataN &...dataN);

    template <typename Address, typename Data1, typename... DataN>
    void safeload_write(const Address& address, const Data1& data1, const DataN &...dataN)
    {
        // Store passed address
        _dspRegAddr = address;
        safeload_write_wrapper(data1, dataN...);
    }

    /***************************************
    Function: safeload_writeRegister()
    Purpose:  Writes 5 bytes of data to the parameter memory of the DSP, the first byte is 0x00
    Inputs:   uint16_t startMemoryAddress;   DSP memory address
              *data;                 Data array to write
              bool finished;                 Indicates if this is the last packet or not
    Returns:  None
    ***************************************/
    void safeload_writeRegister(uint16_t memoryAddress, uint8_t* data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress, int32_t data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress, float data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress, int16_t data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress, uint32_t data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress, uint16_t data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress, uint8_t data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress, double data, bool finished);


private:
    // Wrapper template functions for safeload template
    template <typename Data1, typename... DataN>
    void safeload_write_wrapper(const Data1& data1, const DataN &...dataN)
    {
        safeload_writeRegister(_dspRegAddr, data1, false);
        _dspRegAddr++;
        safeload_write_wrapper(dataN...);  // Recursive call using pack expansion syntax
    }
    // Handles last argument
    template <typename Data1>
    void safeload_write_wrapper(const Data1& data1)
    {
        safeload_writeRegister(_dspRegAddr, data1, true);
    }

    // Private variables
    uint16_t _dspRegAddr;      // Used by template safeload functions
};
