#include <RCSwitch.h>

RCSwitch rfTransmitter = RCSwitch();

#define SENS_SOIL_MOIST 0 // Using Analog Pin 0
#define RF_TRANSMITTER 10 // Using Pin #10
#define RF_CODE_DRY 1010  
#define RF_CODE_WET 1020  



int moistVal= 0;
int lastMoistVal= 0;

void setup() {
  //Serial.begin(9600);
  rfTransmitter.enableTransmit(RF_TRANSMITTER);  
}

void loop() {

  moistVal = analogRead(SENS_SOIL_MOIST);
  //Serial.println(moistVal);

  if(moistVal > 0 && moistVal != lastMoistVal){

    rfTransmitter.send(moistVal, 24);
    lastMoistVal = moistVal;
  }
  
  delay(43200000UL);//12 ore  
}
