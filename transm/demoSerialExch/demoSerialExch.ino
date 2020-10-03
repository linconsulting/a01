#include <SoftwareSerial.h>
#include <AGPSMessage.h>

#define rxPin 2
#define txPin 3


SoftwareSerial btSerial =  SoftwareSerial(rxPin, txPin);
//AGPSMessage exchMsg = AGPSMessage();
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
    
    while (btSerial.available() > 0){
        
        incomingByte = btSerial.read();       
        
        Serial.write(incomingByte);
        //Serial.write("\n");
        
        
        delay(2);                
        
    }
    

    delay(10);
 

}
