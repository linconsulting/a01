/*
AGPSMessageSender.h - Library for Arduino General Purpose Serial Messaging.
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
#include "AGPSMessageSender.h"
#define parity(b) ((((((b)^(((b)<<4)|((b)>>4))) + 0x41) | 0x7C ) +2 ) & 0x80) //parity logica even: 1 pari 0 dispari



boolean AGPSMessageSender::sendOK(SoftwareSerial &serial){
    
    //byte nByteBuff = serial.println(F("E000000000000000"));    
    //return (nByteBuff > 0) ? true : false;
    setDefaultValue();
    
    //inizio codifica primo bit (lsb) del primo byte    
    bitSet(iMsg[index],0); //start of message

    paramValueIsComplete = true;
    setMessageIsComplete();

    paramValueIsNumeric = false;
    setMessageValueIsNumeric();

    paramValueType = 0;
    setMessageValueType();

    paramValueSign = 1;
    setMessageValueSign();

    paramValueHasPayload = 0;
    setMessageHasPayload();
   
    index++;
    //secondo byte
    if(paramValueHasPayload){
        setMessageValueLength();
        setMessageCommaIndex();
        index++;
    }

    setMessageParamCode();
    
    // inserire valore ed il byte di chiusura



    
}

void AGPSMessageSender::setMessageParamCode(){

    byte buffer;
    iMsg[index] = paramCode << 4;                

    /*
    Given the number 789 :
    9 is 789 % 10
    8 is 789 / 10 % 10
    7 is 789 / 100 % 10    
    */

    buffer = paramCodeNumber / 100 % 10;
    iMsg[index] = iMsg[index] ^ buffer;
    index++;    
    
    buffer = paramCodeNumber / 10 % 10;
    iMsg[index] = buffer << 4;    
    buffer = paramCodeNumber % 10;
    iMsg[index] = iMsg[index] ^ buffer;


}

void AGPSMessageSender::setMessageValueLength(){

    if(paramValueHasPayload && paramValueLength){
        iMsg[index] = paramValueLength << 4;                
    }

}

void AGPSMessageSender::setMessageCommaIndex(){

    if(paramValueHasPayload){        
        iMsg[index] = iMsg[index] ^ paramValueCommaIndex;
    }

}



void AGPSMessageSender::setMessageIsComplete(){

    if(!paramValueIsComplete){
        bitSet(iMsg[index],1); //messaggio incompleto: da accodarne un altro
    }

}

void AGPSMessageSender::setMessageValueIsNumeric(){

    if(!paramValueIsNumeric){
        bitSet(iMsg[index],2); //alfanumerico
    }

}

void AGPSMessageSender::setMessageValueType(){

    switch (paramValueType)
    {
        //0 = byte -> lascia tutti i bit spenti
        case 1:
            bitSet(iMsg[index],3);
            break;
        case 2:
            bitSet(iMsg[index],4);
            break;
        case 3:
            bitSet(iMsg[index],3);
            bitSet(iMsg[index],4);
            break;
        
    }

}

void AGPSMessageSender::setMessageValueSign(){

    if(paramValueSign){
        bitSet(iMsg[index],5); //segno algebrico positivo
    }

}

void AGPSMessageSender::setMessageHasPayload(){

    if(paramValueHasPayload){
        bitSet(iMsg[index],6); //se oltre al codice parametro c'è un valore
    }

}




