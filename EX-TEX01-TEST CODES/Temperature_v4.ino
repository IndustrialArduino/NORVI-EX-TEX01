/*Example Program for NORVI Thermocuple Expansion Module
 * SC18IS602B.cpp of  SC18IS602B Library needs editing
 * I2C address = 0x2F;
 * Wire.begin() to Wire.begin(16,17);
 * 
 * 
 * 
 */


#include "Arduino.h"
#include "SC18IS602B.h"

#define MAX31855_CONVERSION_POWER_UP_TIME   200    //in milliseconds
#define MAX31855_CONVERSION_TIME            100    //in milliseconds, 9..10Hz sampling rate 
#define MAX31855_THERMOCOUPLE_RESOLUTION    0.25   //in °C per dac step
#define MAX31855_COLD_JUNCTION_RESOLUTION   0.0625 //in °C per dac step


#define MAX31855_ID                         31855
#define MAX31855_FORCE_READ_DATA            7      //force to read the data, 7 is unique because d2d1d0 can't be all high at the same time
#define MAX31855_ERROR                      2000   //returned value if any error happends

#define MAX31855_THERMOCOUPLE_OK            0
#define MAX31855_THERMOCOUPLE_SHORT_TO_VCC  1
#define MAX31855_THERMOCOUPLE_SHORT_TO_GND  2
#define MAX31855_THERMOCOUPLE_NOT_CONNECTED 3
#define MAX31855_THERMOCOUPLE_UNKNOWN       4

SC18IS602B spiBridge;
unsigned int b = 0;
const int slaveNum = 1;
const uint8_t spiData[] = { 0x00, 0x00, 0x00, 0x00};
uint8_t spiReadBuf[sizeof(spiData)];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(200);

  //Startup I2C interface on D1 (SDA) and D2 (SCL)
  //for the ESP8266
  spiBridge.begin();
  delay(100);
  spiBridge.configureSPI(false, SC18IS601B_SPIMODE_0, SC18IS601B_SPICLK_1843_kHz);
  delay(100);
//  


  
}

void loop() {
  // put your main code here, to run repeatedly:
Serial.print("CJ1  ");
Serial.print(ColdJunctionTemperature(1)); Serial.print("  TP 1  ");
Serial.println(ReadTemperature(1));
delay(3000); 
Serial.print("CJ0  ");
Serial.print(ColdJunctionTemperature(0)); Serial.print("  TP 0  ");
Serial.println(ReadTemperature(0));
Serial.println(" " );
delay(3000); 
 
}


int32_t readRawData(unsigned int chip){
  int32_t rawData = 0;
   
  bool ok = spiBridge.spiTransfer(chip, spiData, sizeof(spiData), spiReadBuf);
  rawData = spiReadBuf[0];
  rawData <<= 8;
  rawData |= spiReadBuf[1];
  rawData <<= 8;
  rawData |= spiReadBuf[2];
  rawData <<= 8;
  rawData |= spiReadBuf[3];
  
  //rawData = spiReadBuf[0]|spiReadBuf[1]<<8|spiReadBuf[2]<<16|spiReadBuf[3]<<24;
  return rawData;
}

float ColdJunctionTemperature(unsigned int chip)
{
  int32_t rawValue = readRawData(chip);

  rawValue = rawValue & 0x0000FFFF;                                   //clear D31..D16 bits
  rawValue = rawValue >> 4;                                           //clear D3...D0  bits
  //rawValue = rawValue * 0.25;
  return (float)rawValue * MAX31855_COLD_JUNCTION_RESOLUTION;
  //return (float)rawValue;
}

float ReadTemperature(unsigned int chip)
{
  int32_t rawValue = readRawData(chip);

  //if (detectThermocouple(rawValue) != MAX31855_THERMOCOUPLE_OK ) return MAX31855_ERROR;
  rawValue = rawValue >> 18; //clear D17..D0 bits
  double centigrade = rawValue;
  centigrade *= 0.25;
  //return (float)rawValue * MAX31855_THERMOCOUPLE_RESOLUTION;
  return centigrade;
}

uint8_t detectThermocouple(unsigned int chip)
{
  int32_t rawValue = readRawData(chip);

  if (bitRead(rawValue, 16) == 1)
  {
    if      (bitRead(rawValue, 2) == 1) return MAX31855_THERMOCOUPLE_SHORT_TO_VCC;
    else if (bitRead(rawValue, 1) == 1) return MAX31855_THERMOCOUPLE_SHORT_TO_GND;
    else if (bitRead(rawValue, 0) == 1) return MAX31855_THERMOCOUPLE_NOT_CONNECTED;
    else                                return MAX31855_THERMOCOUPLE_UNKNOWN;
  }
  return MAX31855_THERMOCOUPLE_OK;
}
