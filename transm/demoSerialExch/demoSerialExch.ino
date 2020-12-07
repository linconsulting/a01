#include <SoftwareSerial.h>
#include <AGPSMessage.h>
#include <AGPSMessageReceiver.h>

#define rxPin 2
#define txPin 3


SoftwareSerial btSerial =  SoftwareSerial(rxPin, txPin);
AGPSMessageReceiver exchMsg = AGPSMessageReceiver();
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
    
    if(exchMsg.readFromSerial(btSerial, Serial)){
      
      
      Serial.write("\n");            
      Serial.println(exchMsg.paramCode,DEC);      
      Serial.println(exchMsg.paramValueType,DEC);

      for(int j = 0; j < 9; j++){        
        Serial.write(exchMsg.paramValue[j]);
      }

      Serial.write("\n");      

      Serial.println(exchMsg.paramValueCommaIndex,DEC);      

      Serial.println(exchMsg.paramValueIsComplete,DEC);

      

    }

    delay(10);

  

}