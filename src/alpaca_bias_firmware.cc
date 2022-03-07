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
 *  2022-03-07
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
#define DEFAULT_POT_WIPER 1
#define MIN_TOLERANCE 10.0 //minimum acceptable difference between desired and realized bias current

void setWiper(uint8_t wiper, uint8_t val);
void printCV();
float getLoadV();
float getCurrent();

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
    while (Serial.available() > 0) 
        Serial.read();
    
    Serial.println("(built: " + String(__DATE__) + "_" + String(__TIME__) + " )\r");
    Serial.println("Select Option:\r\n1. Read Voltage(V),Current(mA)\r\n2. Set Wiper\r\n3. Set Current");
    int cmd = cons->getInt("\r\noption: ");
    int wiper = 0, wiperval = 0;
    float cur; //desired current

    switch (cmd)
    {
    case 1: // REad out Voltage, current
        printCV();
        break;
    case 2: // set resistance on POT
        wiper = (uint8_t) cons->getInt("wiper (1-4): ");
        if (wiper < 1 || wiper > 4){
            Serial.println("Invalid Wiper pick 1 - 4\r\n");
            return;
        }
        wiperval = (uint8_t) cons->getInt("Value (0-255): ");
        setWiper(wiper-1, wiperval);
        
        break;
    case 3: //set and keep current. 
        /**
         * This will probably have to be it's own loop until broken out by user input
         *  current seaking algorithm....
         *  Basically need to set a wiper value, check the current and adjust accordingly
         *  S1, should set wiper to _, to represent the minumum current
         *  S2. climb up/down the wiper value until we are within some tolerance of the desired 'Current' value
         *  S3. break out
         * 
         */
        cur = (float) cons->getDouble("current (mA): ");
        setWiper(DEFAULT_POT_WIPER, 0); //START AT MIN VAL
        Serial.println("\r\n");
        for (int i = 0; i < 256; i++){
            Serial.println("Trying wiper("+String(DEFAULT_POT_WIPER)+")=" + String(i) + "\r");
            float x = 0;
            setWiper(DEFAULT_POT_WIPER, i);
            x = getCurrent();

            if (abs(cur-x) <= MIN_TOLERANCE)
            {
                Serial.println("\r\nfound setting, measured=" + String(abs(cur-x)) +  " wiper=" + String(i) + "\r\n");
                break;
            }
            
            delay(250);
            //break out of this thing and return to menu if anything is sent over serial
            if(Serial.available() > 0)
                return;
        
            
        }


        
        break;
    default:
        Serial.println("\r\n");
        break;
    }

}

void setWiper(uint8_t wiper, uint8_t val){
    int status = 0;
    
    Wire.beginTransmission(AD5144_I2C_ADDR); 

    Wire.write(AD5144_CMD_WRITE_RDAC + wiper); //Write to wiper/RDAC1
    Wire.write(val); //write value to chip
    
    status = Wire.endTransmission();
    if(status != 0)
        Serial.println("\r\nError, couldn't communicate with AD5144 chip over i2c status=" + String(status) + "\r\n");
    
}



float getCurrent(){
    return ina219.getCurrent_mA();
}

float getLoadV(){
    float shuntvoltage = 0,
    busvoltage = 0,
    loadvoltage = 0;

    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    loadvoltage = (busvoltage + (shuntvoltage / 1000));
    return loadvoltage;
}

// Print voltage,current in volts,miliAmp\n format
void printCV(){
    Serial.println("\r\n" + String(getLoadV()) + "," + String(getCurrent()) + "\r\n");
}
