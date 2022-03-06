#include <Arduino.h>
#include "TuiConsole.h"
#include <Adafruit_INA219.h>
#include <Wire.h>

Adafruit_INA219 ina219;
TuiConsole *cons;

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

}