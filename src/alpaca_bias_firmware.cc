/**
 * @file alpaca_bias_firmware.cc
 * @author carobers@asu.edu
 * @brief bias board control software
 * @version 0.1
 * @date 2022-03-05
 * 
 * @copyright Copyright (c) 2022 Arizona State University
 * 
 * REVISIONS:
 *  2022-03-05
 *      FILE CREATED
 */

#include <Arduino.h>
#include "TuiConsole.h"
#include <Adafruit_INA219.h>
#include <Wire.h>
#include <assert.h>

Adafruit_INA219 ina219;
TuiConsole *cons;

/**  spec'd from AD5144 datasheet
 * SEE: https://www.analog.com/media/en/technical-documentation/data-sheets/AD5124_5144_5144A.pdf
 **/

#define AD5144_I2C_ADDR 0b0101111 //ADDR 0 -> GND   ADDR 1 -> GND (AAD5144 DATASHEET)
#define AD5144_CMD_WRITE_RDAC 0b00010000 //offset +1 for each RDAC register

void setWiper(uint8_t wiper, uint8_t val);
void printCV();

void setup(){
    cons = new TuiConsole(&Serial, 9600); //Setup Serial Console

    // Toggle () pin
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    
    // Toggle () pin
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);

    Wire.begin(); //start I2C bus
    ina219.begin(); //Default current/voltage sensor init
}

void loop(){
    Serial.println("Select Option:\n1. Read Voltage(V),Current(mA)\n2. Set Current");
    int cmd = cons->getInt("option->");

    switch (cmd)
    {
    case 1:
        printCV();
        break;
    case 2:
        //TODO: implement this
        break;
    default:
        Serial.println("Invalud CMD");
        break;
    }

}

void setWiper(uint8_t wiper, uint8_t val){
    int status = 0;
    assert(wiper < 4); //should never be programmed past 3 (for RDAC4)
    
    Wire.beginTransmission(AD5144_I2C_ADDR); 

    Wire.write(AD5144_CMD_WRITE_RDAC + wiper); //Write to wiper/RDAC1
    Wire.write(val); //whatever 1111_1111 means to the chip
    
    status = Wire.endTransmission();
    if(status != 0)
        Serial.println("Error, couldn't communicate with AD5144 chip over i2c status=" + String(status));
    
}

// Print voltage,current
void printCV(){
    float shuntvoltage = 0,
        busvoltage = 0,
        current_mA = 0,
        loadvoltage = 0;
    
    shuntvoltage = ina219.getShuntVoltage_mV() + 0.26;
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    loadvoltage = (busvoltage + (shuntvoltage / 1000));

    Serial.println(String(loadvoltage) + "," + String(current_mA));
}
