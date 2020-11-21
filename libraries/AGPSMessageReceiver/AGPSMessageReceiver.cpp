/*
AGPSMessageReceiver.h - Library for Arduino General Purpose Serial Messaging.
Created by Giacomo Solazzi, November 21, 2020.
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
#include "AGPSMessageReceiver.h"
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

boolean AGPSMessage::rFS(SoftwareSerial &serial, HardwareSerial &serialOut){

    
    //while (serial.available() > 0){
    //    iByte = serial.read();
    //    serialOut.write(iByte+"\n");      
    //}
    return true;

}


boolean AGPSMessage::writeOkToSerial(SoftwareSerial &serial){
    
    byte nByteBuff = serial.println(F("E000000000000000"));
    
    return (nByteBuff > 0) ? true : false;
    
    
}


boolean AGPSMessage::readFromSerial(SoftwareSerial &serial){
    
    
    if(serial.available() > 0){
        setDefaultValue();
    }

    boolean vRet = false;

    while (serial.available() > 0){
        
        iByte = serial.read();
        
        if(index < maxInputByte || iByte == '\0'){
            iMsg[index] = iByte;
            vRet = true;
        }        

        delay(2);                
        index++;        
    }

    if(vRet){
        return decodeMsg();
    }

    
    return vRet;

}



boolean AGPSMessage::readFromSerial(HardwareSerial &serial){

    if(serial.available() > 0){
        setDefaultValue();
    }

    boolean vRet = false;

    //while (serial.available() > 0){
    //    
    //    iByte = serial.read();
    //    
    //    if(index <= maxInputChar || iByte == '\0'){
    //        setCharMsg(index, iByte);            
    //        vRet = true;
    //    }        
//
    //    delay(2);                
    //    index++;        
    //}
    
    return vRet;

}


boolean AGPSMessage::decodeMsg(){

    //index è l'indice del byte da decodificare
    //nell'array iMsg
    index = 0;    
    if(!decodeFirstByte()){
        return false;
    }        
    decodeSecondByte();
    decodeThirdFourthBytes();
    decodeValue();    
    return checkEOM();            

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



boolean AGPSMessage::decodeFirstByte(){

    //inizio decodifica primo byte, primo bit (lsb)
    if(bitRead(iMsg[index],0) == 0){
        return false;
    }
    
    paramValueIsComplete = true;
    if(bitRead(iMsg[index],1)){
        paramValueIsComplete = false;        
    }
    
    paramValueIsNumeric = true;
    if(bitRead(iMsg[index],2)){
        paramValueIsNumeric = false;        
    }

    if(!paramValueIsNumeric){
        paramValueType = 0;
        paramValueSign = 0;
    }else{
        decodeBits(3,5);
        if(byteDecoded >= 0 && byteDecoded <= 3){
            paramValueType = byteDecoded;
        }        
        paramValueSign = bitRead(iMsg[index],0);        
    }
    
    paramValueHasPayload = false;
    if(bitRead(iMsg[index],6)){
        paramValueHasPayload = true;        
    }

    //bit 8 msb nn contiene informazione
    //fine decodifica primo byte
    

}

void AGPSMessage::decodeSecondByte(){
    
    index++;

    if(paramValueHasPayload){
        decodeBits(0,5);
        if(byteDecoded >= 0 && byteDecoded <= 9){
            paramValueLength = byteDecoded;
        }
        decodeBits(5);
        if(byteDecoded >= 0 && byteDecoded <= 9){
            paramValueCommaIndex = byteDecoded;
        }
    }else{
        paramValueLength = 0;
        paramValueCommaIndex = 0;
    }

}

void AGPSMessage::decodeThirdFourthBytes(){

    byte bufferByte = 0;

    index++;    
    decodeBits();    
    paramCode = byteDecoded;
    index++; 
    decodeBits();
    bufferByte = byteDecoded;
    
    byteDecoded = paramCode;
    paramCode = byteDecoded >> 4;
    
    byteDecoded <<= 4;    
    paramCodeNumber = (byteDecoded >> 4) * 100; //prima cifra parametro    
    paramCodeNumber += (bufferByte >> 4) * 10; //incremento con seconda cifra parametro

    byteDecoded = bufferByte << 4;
    paramCodeNumber += byteDecoded >> 4; //incremento con terza cifra parametro

}

void AGPSMessage::decodeValue(){

    index++;
    decodeBits();
    byte bufferByte = 0;

    if(paramValueLength > 0){

        bufferByte = !parity(paramValueIsNumeric);

        if(paramValueIsNumeric){

            bufferByte = paramValueLength / 2;
            
            if(bufferByte != 0){
                lsbIsNibble = true;
                bufferByte++;
            }
            

        }else
        {
            bufferByte = paramValueLength;
        }
        
        paramValue[0] = byteDecoded;
        paramValue[1] = '\0';
        
        for (byte i = 1; i <= bufferByte; i++)
        {
            index++;
            decodeBits();
            paramValue[i] = byteDecoded;
            paramValue[i+1] = '\0';
            
        }
        
    }


}

boolean AGPSMessage::checkEOM(){

   if(lsbIsNibble){
       byteDecoded <<= 4;    
       byteDecoded >>= 4;
   }
   return byteDecoded == 4 ? true : false;
    
}


float AGPSMessage::getValueInFloat(){

    float vRet = 0.0;
    int cntExp = -paramValueCommaIndex;
    index = 0;

    for(int i = sizeof(paramValue)-1; i >= 0; i--){

        if(!paramValueIsNumeric){
            vRet += (paramValue[i]-'0')*pow(10,cntExp);
        }else
        {
            index = (paramValue[i] << 4) >> 4;
            vRet += index*pow(10,cntExp);
            cntExp++;
            index = paramValue[i] >> 4;
            vRet += index*pow(10,cntExp);
        }
        cntExp++;

    }

    if(paramValueSign == 0){
        vRet *= -1;
    }

    return vRet;

}