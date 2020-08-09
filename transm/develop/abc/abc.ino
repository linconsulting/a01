
#include <SoftwareSerial.h>

#define rxPin 2
#define txPin 3

SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

char inData[20]; // Allocate some space for the string
char inChar=-1; // Where to store the character read
byte index = 0; // Index into array; where to store the character


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
  mySerial.begin(9600);
  mySerial.println(F("Abc started."));

  Serial.println(F("Setup avvenuto."));


}


char Comp(char* This) {
    while (mySerial.available() > 0) // Don't read unless
                                   // there you know there is data
    {
        if(index < 19) // One less than the size of the array
        {
            inChar = mySerial.read(); // Read a character
            inData[index] = inChar; // Store it
            index++; // Increment where to write next
            inData[index] = '\0'; // Null terminate the string
        }
    }

    if (strcmp(inData,This)  == 0) {
        for (int i=0;i<19;i++) {
            inData[i]=0;
        }
        index=0;
        return(0);
    }
    else {
        return(1);
    }
}

void loop() // run over and over
{
  if (mySerial.available())
      Serial.write(mySerial.read());      
  if (Serial.available())
    mySerial.write(Serial.read());
  //if (mySerial.available() && Comp("ciao\n")==0) {
  //      Serial.write("Ricevuta stringa: Ciao\n");
  //}

  //if (mySerial.available()){
  //  while (mySerial.available() > 0){
  //    if(index < 19) // One less than the size of the array
  //      {
  //          inChar = mySerial.read(); // Read a character
  //          inData[index] = inChar; // Store it
  //          index++; // Increment where to write next            
  //          inData[index] = '\0'; // Null terminate the string
  //      }
  //  }
  //  Serial.println(inData);
  //  index = 0;        
  //}


}
