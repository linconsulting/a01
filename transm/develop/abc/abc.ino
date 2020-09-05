
#include <SoftwareSerial.h>
#include <AGPSMessage.h>

#define rxPin 2
#define txPin 3

#define E00 165
#define E01 166
#define S01 180
#define G01 168
#define S02 181
#define G02 169
#define S03 182
#define G03 170
#define S04 183
#define G04 171
#define S05 184
#define G05 172
#define S06 185
#define G06 173
#define S07 186
#define G07 174
#define S08 187
#define G08 175


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

boolean updateVars(AGPSMessage &msg){

  int strCode = 0;
  for(int i = 0; i < sizeof(msg.paramCode); i++){
    strCode += msg.paramCode[i];      
  }
  
  switch (strCode) {
    case S01:

      msg.getValueInUL();
      
      return true;
      break;
    case G01:
      return true;
      break;
    case S02:
      return true;
      break;
    case G02:
      return true;
      break;    
    case S03:
      return true;
      break;
    case G03:
      return true;
      break;
    case S04:
      return true;
      break;
    case G04:
      return true;
      break;
    case S05:
      return true;
      break;
    case G05:
      return true;
      break;
    case S06:
      return true;
      break;
    case G06:
      return true;
      break;
    case S07:
      return true;
      break;
    case G07:
      return true;
      break;
    case S08:
      return true;
      break;
    case G08:
      return true;
      break;
    default:
      return false;
      break;
  }

  return false;

}



void loop() // run over and over
{      
    
    if(exchMsg.readFromSerial(btSerial)){
      
      Serial.write("\n");

      for(int i = 0; i < 3; i++){
        Serial.write(exchMsg.paramCode[i]);      
      }
      
      Serial.write("\n");      

      Serial.write(exchMsg.paramValueType + '0');

      Serial.write("\n");      

      for(int j = 0; j < 10; j++){        
        Serial.write(exchMsg.paramValue[j]);
      }

      Serial.write("\n");      

      Serial.write(exchMsg.paramValueCommaIndex + '0');

      Serial.write("\n"); 

      Serial.write(exchMsg.paramValueIsComplete + '0');

      boolean r = exchMsg.writeOkToSerial(btSerial);

      Serial.write(r + '0');

      Serial.write("\n"); 

      Serial.write(updateVars(exchMsg) + '0');

    }

    delay(10);

  

}
