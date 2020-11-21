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
#define parity(b) ((((((b)^(((b)<<4)|((b)>>4))) + 0x41) | 0x7C ) +2 ) & 0x80) //parity logica even: 1 pari 0 dispari


void AGPSMessage::setDefaultValue(){

    paramValueType = 255;
    memset(paramValue, ' ', sizeof(paramValue));  
    paramValueCommaIndex = 255;
    paramValueIsComplete = 255;

    index = 0;
    msgLength = 0;
    lsbIsNibble = false;
    
}


void AGPSMessage::decodeByte(){

    byteDecoded = 0;
    
    for (byte i = 0; i < 8; i++)
    {
        if(bitRead(iMsg[index],i)){
            bitSet(byteDecoded,i);
        }        
        
    }


}

void AGPSMessage::decodeBits(byte bitFrom=0, byte bitTo=8, byte bitSetFrom = 0){

    byteDecoded = 0;
    byte j = bitSetFrom;
    
    for (byte i = bitFrom; i < bitTo; i++)
    {
        if(bitRead(iMsg[index],i)){
            bitSet(byteDecoded,j);
        }        
        j++;
    }

    
}



