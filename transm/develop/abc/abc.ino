
#include <SoftwareSerial.h>
#include <AGPSMessage.h>

#define rxPin 2
#define txPin 3

SoftwareSerial btSerial =  SoftwareSerial(rxPin, txPin);

//char inData[20]; // Allocate some space for the string
//char inChar=-1; // Where to store the character read
//byte index = 0; // Index into array; where to store the character

AGPSMessage exchMsg = AGPSMessage();
char x = ' ';

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
  //if (btSerial.available())
  //    Serial.write(btSerial.read());      
  //if (Serial.available())
  //  btSerial.write(Serial.read());
  
      
    if(exchMsg.readFromSerial(btSerial, Serial)){
      
      Serial.write("\n");

      for(int i = 0; i < 3; i++){
        Serial.write(exchMsg.paramCode[i]);      
      }
      
      Serial.write("\n");      

      for(int j = 0; j < 10; j++){        
        Serial.write(exchMsg.paramValue[j]);
      }

      Serial.write("\n");      

      Serial.write(exchMsg.paramValueIsComplete + '0');


    }

    
  

  //if(exchMsg.rFS(btSerial, Serial)){
  //  Serial.write("OK\n");      
  //  delay(500);
  //}

}
