/*
AGPSMessage.h - Library for Arduino General Purpose Serial Messaging.
Created by Giacomo Solazzi, August 9, 2020.
Released into the public domain.
For more information: variable declaration, changelog,... see AGPSMessage.h

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <Arduino.h>
#include "AGPSMessage.h"
//#include "defines.h"
#include "string.h"



AGPSMessage::AGPSMessage()
{
  memset(paramCode, 0, sizeof paramCode);  
    
}

boolean AGPSMessage::readFromSerial(SoftwareSerial &serial){

    count = 0;
    index = 0;

    while (serial.available() > 0){

        inChar = serial.read();
        if(index < maxInputChar){
            setCharMsg(index, inChar);
        }        
        index++;
    }

}

boolean AGPSMessage::readFromSerial(HardwareSerial &serial){

    count = 0;
    index = 0;

    while (serial.available() > 0){

        inChar = serial.read();
        if(index < maxInputChar){
            setCharMsg(index, inChar);
        }        
        index++;
    }

}

void AGPSMessage::setCharMsg(byte index, char value){

    if(index < sizeof(paramCode))   // <3
    {            
        paramCode[index] = value;        
    }

    if(index == sizeof(paramCode)) // 3
    {
        paramValueType = value - '0';        
    }

    if(index > sizeof(paramCode) && index <= (sizeof(paramCode)+sizeof(paramValue)) ) //<= 13
    {
        paramValue[count] = value;
        count++;            
    }

    if(index == (sizeof(paramCode)+sizeof(paramValue) + 1)) // 14
    {        
        paramValueCommaIndex = value - '0';              
    }

    if(index == (sizeof(paramCode)+sizeof(paramValue) + 2)) // 15
    {        
        paramValueIsComplete = value - '0';              
    }


}