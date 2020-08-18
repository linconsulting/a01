
#include <SoftwareSerial.h>
#include <AGPSMessage.h>

#define rxPin 2
#define txPin 3

SoftwareSerial btSerial =  SoftwareSerial(rxPin, txPin);

//char inData[20]; // Allocate some space for the string
//char inChar=-1; // Where to store the character read
//byte index = 0; // Index into array; where to store the character

AGPSMessage exchMsg = AGPSMessage();


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
  

  //if (btSerial.available()){
  //  while (btSerial.available() > 0){
  //    if(index < 19) // One less than the size of the array
  //      {
  //          inChar = btSerial.read(); // Read a character
  //          inData[index] = inChar; // Store it
  //          index++; // Increment where to write next            
  //          inData[index] = '\0'; // Null terminate the string
  //      }
  //  }
  //  Serial.println(inData);
  //  index = 0;        
  //}
  
  delay(500);
  
  if(exchMsg.readFromSerial(btSerial)) {
    
    Serial.write(exchMsg.paramCode);      
    Serial.write("\n");      
    Serial.write(exchMsg.paramValue);
  
  }

  //if(exchMsg.rFS(btSerial, Serial)){
  //  Serial.write("OK\n");      
  //  delay(500);
  //}

}
