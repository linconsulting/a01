#include <SoftwareSerial.h>
#include <AGPSMessage.h>
#include <AGPSMessageReceiver.h>
#include <AGPSMessageSender.h>

#define rxPin 2
#define txPin 3


SoftwareSerial btSerial =  SoftwareSerial(rxPin, txPin);
AGPSMessageReceiver iMsg = AGPSMessageReceiver();
AGPSMessageSender oMsg = AGPSMessageSender();

int incomingByte;   

void setup()
{

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
  
  //the SoftwareSerial port:  
  btSerial.begin(9600);
  btSerial.println(F("Abc started."));
  Serial.println(F("Setup avvenuto."));

}




void loop() // run over and over
{      
    
    if(iMsg.readFromSerial(btSerial, Serial)){
      
      
      Serial.write("\n");            
      
      switch (iMsg.paramCode)
      {
      case 0:
        Serial.write("G");
        break;
      case 1:
        Serial.write("S");
        break;
      case 2:
        Serial.write("E");
        break;
      case 3:
        Serial.write("W");
        break;
      case 4:
        Serial.write("M");
        break;
      }
      
      Serial.println(iMsg.paramCodeNumber,DEC);      
      
      oMsg.sendOK(btSerial);

      for (byte i = 0; i < 4; i++)
      {
        Serial.println(oMsg.iMsg[i],BIN);
      }
      
      
      
      
      
    }

    delay(10);

  

}