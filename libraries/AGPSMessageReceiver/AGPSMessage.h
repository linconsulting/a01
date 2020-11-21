/*
  AGPSMessageReceiver.h - Library for Arduino General Purpose Serial Messaging.
  Created by Giacomo Solazzi, November 21, 2020.
  Released into the public domain.

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
#ifndef AGPSMessage_h
#define AGPSMessage_h

#include "Arduino.h"
#include <SoftwareSerial.h>

class AGPSMessage
{
  public:
    
    boolean readFromSerial(SoftwareSerial &serial);    
    boolean readFromSerial(HardwareSerial &serial);
    
    boolean writeOkToSerial(SoftwareSerial &serial);            

    boolean rFS(SoftwareSerial &serial, HardwareSerial &serialOut);

    float getValueInFloat();
    
    
    byte paramCode;    
    int paramCodeNumber;    
    byte paramValueType;
    byte paramValueSign;
    byte paramValueLength;
    char paramValue[9];
    byte paramValueCommaIndex;
    bool paramValueIsComplete;
    bool paramValueIsNumeric;
    bool paramValueHasPayload;
    byte byteDecoded;

    

  private:
    void setDefaultValue();    
    void decodeSecondByte();
    void decodeThirdFourthBytes();
    void decodeValue();
    
    boolean decodeMsg();   
    boolean decodeFirstByte();

    boolean checkEOM();   //end of message
    byte decodeByte();
    byte decodeBits(byte bitFrom, byte bitTo, byte bitSetFrom); 

    byte iMsg[10];
    int iByte;
    byte index, msgLength;
    bool lsbIsNibble;
    const byte maxInputByte = 14;
};

#endif