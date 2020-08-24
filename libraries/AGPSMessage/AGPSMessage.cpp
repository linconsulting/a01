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



void AGPSMessage::setDefaultValue(){

    memset(paramCode, ' ', sizeof(paramCode));  
    paramValueType = 255;
    memset(paramValue, ' ', sizeof(paramValue));  
    paramValueCommaIndex = 255;
    paramValueIsComplete = 255;

    count = 0;
    index = 0;
    
}

boolean AGPSMessage::rFS(SoftwareSerial &serial, HardwareSerial &serialOut){

    
    //while (serial.available() > 0){
    //    inChar = serial.read();
    //    serialOut.write(inChar+"\n");      
    //}
    return true;

}

boolean AGPSMessage::readFromSerial(SoftwareSerial &serial, HardwareSerial &serialOut){
    
    
    if(serial.available() > 0){
        setDefaultValue();
    }

    boolean vRet = false;

    while (serial.available() > 0){
        
        inChar = serial.read();
        
        if(inChar == '\0' || index > maxInputChar){
            vRet = true;
        }
        else if(index <= maxInputChar){
            setCharMsg(index, inChar);
            vRet = true;
        }

        delay(2);                
        index++;
        vRet = true;
    }
    
    return vRet;

}

boolean AGPSMessage::readFromSerial(SoftwareSerial &serial){

    setDefaultValue();

    while (serial.available() > 0){

        inChar = serial.read();
        if(index < maxInputChar){
            setCharMsg(index, inChar);
        }else
        {            
            return true;
        }
                
        index++;
    }

    return false;

}

boolean AGPSMessage::readFromSerial(HardwareSerial &serial){

    setDefaultValue();

    while (serial.available() > 0){

        inChar = serial.read();
        if(index < maxInputChar){
            setCharMsg(index, inChar);
        }else
        {
            break;
        }
                
        index++;
    }

    return true;

}

void AGPSMessage::setCharMsg(byte index, char value){

    if(index < sizeof(paramCode))   // <3
    {            
        paramCode[index] = value;        
        paramCode[index+1] = '\0';        
    }

    if(index == sizeof(paramCode)) // 3
    {
        paramValueType = value - '0';        
    }

    if(index > sizeof(paramCode) && index <= (sizeof(paramCode)+sizeof(paramValue)) ) //<= 13
    {
        paramValue[count] = value;
        paramValue[count+1] = '\0';
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