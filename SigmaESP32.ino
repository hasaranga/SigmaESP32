
// By R.Hasaranga
// http://www.hasaranga.com
// Code may not clean, but it works :-)

#include <WiFi.h>
#include <Wire.h>
#include "DSPWriter.h"

const char* ssid     = "your router ssid";
const char* password = "your router password";

//Static IP address configuration
IPAddress staticIP(192, 168, 8, X); //static ip address (change according to your router config)
IPAddress gateway(192, 168, 8, 1);   //IP sddress of WiFi router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS

WiFiServer tcpServer(8086);

unsigned char dataBuffer[50*1024];

#define STATE_START 0
#define STATE_READ_CMD 1
#define STATE_WRITE_CMD 2

#define CMD_WRITE 0x09
#define CMD_READ 0x0a

struct adauWriteHeader {
  uint8_t command;
  uint8_t safeload;
  uint8_t placement;
  uint16_t totalLen;
  uint8_t chipAddr;
  uint16_t dataLen;
  uint16_t address;
};

struct adauReadHeader {
  uint8_t command;
  uint16_t totalLen;
  uint8_t chipAddr;
  uint16_t dataLen;
  uint16_t address;
};

adauWriteHeader writeHeader;
adauReadHeader readHeader;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(staticIP, gateway, subnet, dns);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("WiFi connected.");

  Wire.begin();
  tcpServer.begin();
}

void loop() {
  WiFiClient client = tcpServer.available();   // listen for incoming clients
  if (client) {
    Serial.println("new connection");
    int writeIndex = 0;
    int readIndex = 0;
    int receivedByteCount = 0;
    int currentState = STATE_START;
    bool flashingDetected = false;
    int lastRegSize;
    while (client.connected())
    {
      digitalWrite(LED_BUILTIN, HIGH);
      while (client.available())
      {
        dataBuffer[writeIndex] = client.read();
        writeIndex++;
        receivedByteCount++;
      }

      if(currentState == STATE_START)
      {
        if(receivedByteCount > readIndex)
        {
          if(dataBuffer[readIndex] == CMD_READ)
          {
            currentState = STATE_READ_CMD;
          }
          else if(dataBuffer[readIndex] == CMD_WRITE)
          {
            currentState = STATE_WRITE_CMD;
          }
          else
          {
            Serial.println("invalid operation");
            break;
          }
        }
        else
        {
          currentState = STATE_START;
          
          if(writeIndex!=0)
            Serial.println("data processed");
            
          writeIndex = 0;
          readIndex = 0;
          receivedByteCount = 0;   
          flashingDetected = false;           
        }
      }
      else if(currentState == STATE_WRITE_CMD)
      {
        if(receivedByteCount >= (readIndex+sizeof(adauWriteHeader)))
        {          
          writeHeader.safeload = dataBuffer[readIndex+1];
          writeHeader.placement = dataBuffer[readIndex+2];
          writeHeader.totalLen = (dataBuffer[readIndex+3]<<8) | dataBuffer[readIndex+4];
          writeHeader.chipAddr = dataBuffer[readIndex+5];
          writeHeader.dataLen = (dataBuffer[readIndex+6]<<8) | dataBuffer[readIndex+7];
          writeHeader.address = (dataBuffer[readIndex+8]<<8) | dataBuffer[readIndex+9];                   
          readIndex += 10;

          digitalWrite(LED_BUILTIN, LOW);
            
            uint8_t registerSize = writeHeader.dataLen;
            uint16_t regAddress = writeHeader.address;
            
            if((writeHeader.address == dspRegister::CoreRegister) & (writeHeader.dataLen == 2))
            {
              registerSize = CORE_REGISTER_R0_REGSIZE; // CORE_REGISTER_R0_REGSIZE or CORE_REGISTER_R4_REGSIZE
              Serial.println("CORE_REGISTER_R0_REGSIZE");
            }
            else if((writeHeader.address == dspRegister::CoreRegister) & (writeHeader.dataLen == 24))
            {
              registerSize = HARDWARE_CONF_REGSIZE;
              Serial.println("HARDWARE_CONF_REGSIZE");
            }
            else if(writeHeader.address == 0x0400) 
            {
              registerSize = PROGRAM_REGSIZE;
              flashingDetected = true;      
              Serial.println("PROGRAM_REGSIZE");      
            }
            else if(writeHeader.address == 0) 
            {
              registerSize = PARAMETER_REGSIZE;
              Serial.println("PARAMETER_REGSIZE");
            }
            else
            {        
              if(flashingDetected){
                registerSize = lastRegSize;
                Serial.println("UNKNOWN REGSIZE: using last reg size");
              }
            }

           if(writeHeader.safeload == 1)
           {
            Serial.print("safeload data size: ");
            Serial.println(writeHeader.dataLen);
            int writeCount = writeHeader.dataLen/4;
            uint8_t dataArray[5];
            int safeLoadReadIndex = readIndex;
            DSPWriter dspWriter;
            while(writeCount)
            {
              dataArray[0] = 0;
              dataArray[1] = dataBuffer[safeLoadReadIndex];
              dataArray[2] = dataBuffer[safeLoadReadIndex+1];
              dataArray[3] = dataBuffer[safeLoadReadIndex+2];
              dataArray[4] = dataBuffer[safeLoadReadIndex+3];

              
              dspWriter.safeload_writeRegister(regAddress, dataArray, writeCount==1);
              regAddress++;
              safeLoadReadIndex+=4;
              --writeCount;
            }
           }
           else
           {
            DSPWriter::writeRegisterBlock(regAddress, writeHeader.dataLen, &dataBuffer[readIndex], registerSize);
           }
           
           lastRegSize = registerSize;
          
          digitalWrite(LED_BUILTIN, HIGH);

          readIndex += writeHeader.dataLen;
          currentState = STATE_START;
        }
      }
      else if(currentState == STATE_READ_CMD)
      {
        if(receivedByteCount >= (readIndex+sizeof(adauReadHeader)))
        {
          readHeader.totalLen = (dataBuffer[readIndex+1]<<8) | dataBuffer[readIndex+2];
          readHeader.chipAddr = dataBuffer[3];
          readHeader.dataLen = (dataBuffer[4]<<8) | dataBuffer[5];
          readHeader.address = (dataBuffer[6]<<8) | dataBuffer[7];
          readIndex += 8;

          Serial.println("============== read request ============");
        }       
      }   
    }
    client.stop();
    Serial.println("disconnected");
    digitalWrite(LED_BUILTIN, LOW);
  }
  
}
