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
#ifndef AGPSMessageReceiver_h
#define AGPSMessageReceiver_h

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <AGPSMessage.h>

class AGPSMessageReceiver : public AGPSMessage
{
  public:
    
    HardwareSerial *sOut;
    
    boolean readFromSerial(SoftwareSerial &serial, HardwareSerial &serialOut);
    boolean readFromSerial(HardwareSerial &serial);
    boolean rFS(SoftwareSerial &serial, HardwareSerial &serialOut);
    float getValueInFloat();
    

  private:
    void setDefaultValue();    
    void decodeSecondByte();
    void decodeThirdFourthBytes();
    void decodeValue();
    
    boolean decodeMsg();   
    boolean decodeFirstByte();
    boolean checkEOM();   //end of message    
    

    
};

#endif