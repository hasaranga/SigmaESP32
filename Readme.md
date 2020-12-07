# ADAU1401 Wireless Access using ESP32

Connect I2C pin of the ADAU1401 into ESP32 with 2.2K pullups to 3.3v. Also connect 5V and GND.

Use TCPIP1701 component in SigmaStudio to program. Only writing allowed. Reading not implemented. Set maximum buffer size to 1500 in TCPIP settings. (no ASAP connection)

Built in led of the ESP32 will turn on when tcp connection made. If communication error occured, the programmer will disconnect the tcp connection and the led will turn off.

This programmer should work with ADAU1701. But not tested.

Reference: https://github.com/MCUdude/SigmaDSP