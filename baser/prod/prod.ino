#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <RCSwitch.h>

RCSwitch rfReceiver = RCSwitch();

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#define DS3231_ADDRESS 0x68    // RTC is connected, address is Hex68 (Decimal 104)

#define BUTTON_SET 8
#define BUTTON_MOD 7
#define RELAIS_EV_START 9
#define RELAIS_EV_STOP 10
#define BLINK_INT_MODAL_UPD 500
#define MODAL_RUN 100
#define MODAL_IRRIG_ON_P1 110 
#define MODAL_IRRIG_ON_P2 120 
#define MODAL_DEFAULT_PREV_STATUS 200 
#define MODAL_UPD_YY 2
#define MODAL_UPD_MO 3
#define MODAL_UPD_DD 4
#define MODAL_UPD_HH 5
#define MODAL_UPD_MM 6
#define MODAL_START_P1_YY 7
#define MODAL_START_P1_MO 8
#define MODAL_START_P1_DD 9
#define MODAL_START_P1_HH 10
#define MODAL_START_P1_MM 11
#define MODAL_START_P2_YY 12
#define MODAL_START_P2_MO 13
#define MODAL_START_P2_DD 14
#define MODAL_START_P2_HH 15
#define MODAL_START_P2_MM 16
#define MODAL_DURAT_P1_MM 17
#define MODAL_DURAT_P2_MM 18
#define MODAL_FREQ_P1_HH 19
#define MODAL_FREQ_P2_HH 20
#define MODAL_P2_ENABLED 21
#define MODAL_SET_ONLY_PROG 22


#define RF_RECEIVER 0  // Receiver on interrupt 0 => that is pin #2
#define VAL_DRY 0  
#define VAL_WET 1  
#define VAL_MAX_DRY 1023
#define VAL_MAX_WET 280
#define VAL_WET_TRIGGER 550
#define VAL_IMPULSO_EV 200  


int  statusButtonMod = 0;
int  statusButtonSet = 0;
byte currentModalState = 0;
byte previousModalState = 0;
byte lcdStatus = 0;

DateTime now = 0;
uint32_t unixNow;
DateTime startTimeP1 = 0;
DateTime startTimeP2 = 0;
DateTime lastExecution = 0;
DateTime nextExecution = 0;
byte durationP1 = 0;
byte durationP2 = 0;
byte durationShowNext = 15;
byte freqP1 = 0;
byte freqP2 = 0;
byte p2Enabled = 0;
byte setOnlyProg = 0;

int newYY = 2018;
int newMO = 0;
int newDD = 0;
int newHH = 0;
int newMM = 0;
int newSS = 0;

byte soilStatus = 0;


void setup() {

  pinMode(BUTTON_MOD, INPUT);  
  pinMode(BUTTON_SET, INPUT);    
  pinMode(RELAIS_EV_START, OUTPUT);
  pinMode(RELAIS_EV_STOP, OUTPUT);
  lcdOn();
  rtcSetup();
  currentModalState = MODAL_RUN;
  previousModalState = MODAL_DEFAULT_PREV_STATUS;
  
  Serial.begin(9600);  
  rfReceiver.enableReceive(RF_RECEIVER);  
  now = rtc.now();         
  nextExecution = now + TimeSpan(durationShowNext);

}

void loop() {
   
  
  statusButtonMod = digitalRead(BUTTON_MOD);
  statusButtonSet = digitalRead(BUTTON_SET);

  setStatusModal();      
  

  if(currentModalState < MODAL_RUN){
    
    showSettingParams();   
    
    
  }else{

    if(statusButtonSet == HIGH){
        if(lcdStatus == 1){
            lcdOff();
        }else{
            lcdOn();
        }
        delay(2500);
    }
    
    if(lcdStatus == 1){
        digitalClockDisplay();    
    }
    
    receiveRfSignal();    
    ctrlIrrigationStatus();
    setIrrigation();

  }

}


void digitalClockDisplay(){

    now = rtc.now();             
    String strInfo = now.day() > 9 ? String(now.day()) : "0"+String(now.day());
    strInfo += "/";
    strInfo += now.month() > 9 ? String(now.month()) : "0"+String(now.month());
    strInfo += "/";
    strInfo += String(now.year());
    strInfo += " ";
    strInfo += now.hour() > 9 ? String(now.hour()) : "0"+String(now.hour());
    strInfo += ":";
    strInfo += now.minute() > 9 ? String(now.minute()) : "0"+String(now.minute());
    lcd.setCursor(0,0);
    lcd.print(strInfo);            
    
    strInfo = soilStatus == VAL_DRY ? "TA" : "TU";

    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2)){

        strInfo += " - LE ";        
        strInfo += now.hour() > 9 ? String(now.hour()) : "0"+String(now.hour());
        strInfo += ":";
        strInfo += now.minute() > 9 ? String(now.minute()) : "0"+String(now.minute());

    }else if(now.unixtime() < nextExecution.unixtime()){

        strInfo += " - LE ";        
        strInfo += lastExecution.hour() > 9 ? String(lastExecution.hour()) : "0"+String(lastExecution.hour());
        strInfo += ":";
        strInfo += lastExecution.minute() > 9 ? String(lastExecution.minute()) : "0"+String(lastExecution.minute());

    }else{

        strInfo += " - NE ";

        if( startTimeP2.unixtime() == 0 || (startTimeP1.unixtime() < startTimeP2.unixtime())){
            strInfo += startTimeP1.hour() > 9 ? String(startTimeP1.hour()) : "0"+String(startTimeP1.hour());
            strInfo += ":";
            strInfo += startTimeP1.minute() > 9 ? String(startTimeP1.minute()) : "0"+String(startTimeP1.minute());
        }else{
            strInfo += startTimeP2.hour() > 9 ? String(startTimeP2.hour()) : "0"+String(startTimeP2.hour());
            strInfo += ":";
            strInfo += startTimeP2.minute() > 9 ? String(startTimeP2.minute()) : "0"+String(startTimeP2.minute());
        }
       
        nextExecution = (now.unixtime() <= (nextExecution.unixtime() + durationShowNext)) ? nextExecution : now + TimeSpan(durationShowNext);
    }
    
    
    lcd.setCursor(0,1);
    lcd.print(strInfo);
    
}


void ctrlIrrigationStatus(){

    unixNow = now.unixtime();

    if(currentModalState >= MODAL_RUN && unixNow >= startTimeP1.unixtime() && soilStatus == VAL_DRY){
        currentModalState = MODAL_IRRIG_ON_P1;
    }

    if(currentModalState == MODAL_IRRIG_ON_P1 && unixNow >= (startTimeP1.unixtime() + (durationP1*60))){
        currentModalState = MODAL_RUN;
        startTimeP1 = startTimeP1 + TimeSpan(freqP1*3600UL);
        lastExecution = now;
    }

    if(currentModalState >= MODAL_RUN && p2Enabled == 1 && unixNow >= startTimeP2.unixtime() && soilStatus == VAL_DRY){
        currentModalState = MODAL_IRRIG_ON_P2;
    }

    if(p2Enabled == 1 && currentModalState == MODAL_IRRIG_ON_P2 && unixNow >= (startTimeP2.unixtime() + (durationP2*60))){
        currentModalState = MODAL_RUN;
        startTimeP2 = startTimeP2 + TimeSpan(freqP2*3600UL);
        lastExecution = now;
    }

    
    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2) && soilStatus == VAL_WET){
        currentModalState = MODAL_RUN;
        lastExecution = now;
    }


    if(p2Enabled == 0 && currentModalState == MODAL_IRRIG_ON_P2){
        currentModalState = MODAL_RUN;
        startTimeP2 = 0;
    }

}



void rtcSetup(){

  if (! rtc.begin()) {
    lcd.setCursor(0,0);
    lcd.print("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    lcd.setCursor(0,0);
    lcd.print("RTC lost power,");    
    lcd.setCursor(0,1);
    lcd.print("set the time!");    
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2018, 7, 10, 19, 40, 0));
  }
}

void lcdOn(){
  lcd.init();
  lcd.backlight();
  lcdStatus = 1;

}

void lcdOff(){
  lcd.init();
  lcd.noBacklight();
  lcdStatus = 0;

}

void setStatusModal(){

  
  if(statusButtonMod == HIGH){

    switch (currentModalState) {

        case MODAL_SET_ONLY_PROG:
            if(setOnlyProg == 0){
                switchToModalUpdYY();
                
            }else{
                switchtoModalStartP1();      
            }            
                        
        break;
        case MODAL_UPD_YY:
            currentModalState = MODAL_UPD_MO;  
            lcd.setCursor(0, 0);
            lcd.print("SET MONTH       ");
            lcd.setCursor(0, 1);
            lcd.print("MO        ");          
        break;    
        case MODAL_UPD_MO:        
            currentModalState = MODAL_UPD_DD;         
            lcd.setCursor(0, 0);
            lcd.print("SET DAY       ");
            lcd.setCursor(0, 1);
            lcd.print("DD        ");   
        break;
        case MODAL_UPD_DD:        
            currentModalState = MODAL_UPD_HH;     
            lcd.setCursor(0, 0);
            lcd.print("SET HOURS       ");
            lcd.setCursor(0, 1);
            lcd.print("HH        ");       
        break;
        case MODAL_UPD_HH:        
            currentModalState = MODAL_UPD_MM;    
            lcd.setCursor(0, 0);
            lcd.print("SET MINUTES       ");
            lcd.setCursor(0, 1);
            lcd.print("MM        ");        
        break;
        case MODAL_UPD_MM:            
            rtc.adjust(DateTime(newYY, newMO, newDD, newHH, newMM, newSS));
            switchtoModalStartP1();

        break;    
        case MODAL_START_P1_YY:
            currentModalState = MODAL_START_P1_MO;    
            lcd.setCursor(0, 0);
            lcd.print("P1 START MONTH        ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P1_MO:
            currentModalState = MODAL_START_P1_DD;    
            lcd.setCursor(0, 0);
            lcd.print("P1 START DAY        ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P1_DD:
            currentModalState = MODAL_START_P1_HH;    
            lcd.setCursor(0, 0);
            lcd.print("P1 START HOUR        ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P1_HH:
            currentModalState = MODAL_START_P1_MM;    
            lcd.setCursor(0, 0);
            lcd.print("P1 START MINUTES       ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P1_MM:

            startTimeP1 = DateTime(newYY, newMO, newDD, newHH, newMM, newSS);
            newYY = startTimeP1.year() - 1;
            newMO = startTimeP1.month() - 1;
            newDD = startTimeP1.day() - 1;
            newHH = 0;
            newMM = 0;

            currentModalState = MODAL_DURAT_P1_MM;    
            lcd.setCursor(0, 0);
            lcd.print("P1 DURAT MINUTES       ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;        
        case MODAL_DURAT_P1_MM:
            
            currentModalState = MODAL_FREQ_P1_HH;
            lcd.setCursor(0, 0);
            lcd.print("P1 FREQ HOURS       ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_FREQ_P1_HH:

            currentModalState = MODAL_P2_ENABLED;
            lcd.setCursor(0, 0);
            lcd.print("P2 ENABLED         ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;

        case MODAL_P2_ENABLED:

            if(p2Enabled == 0){
                currentModalState = MODAL_RUN;
                lcd.setCursor(0, 0);
                lcd.print("                ");                                    
            }else{
                currentModalState = MODAL_START_P2_YY;    
                lcd.setCursor(0, 0);
                lcd.print("P2 START YEAR        ");             
            }            
            
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;

        case MODAL_START_P2_YY:
            currentModalState = MODAL_START_P2_MO;    
            lcd.setCursor(0, 0);
            lcd.print("P2 START MONTH        ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P2_MO:
            currentModalState = MODAL_START_P2_DD;    
            lcd.setCursor(0, 0);
            lcd.print("P2 START DAY        ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P2_DD:
            currentModalState = MODAL_START_P2_HH;    
            lcd.setCursor(0, 0);
            lcd.print("P2 START HOUR        ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P2_HH:
            currentModalState = MODAL_START_P2_MM;    
            lcd.setCursor(0, 0);
            lcd.print("P2 START MINUTES       ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_START_P2_MM:

            startTimeP2 = DateTime(newYY, newMO, newDD, newHH, newMM, newSS);
            newYY = startTimeP2.year() - 1;
            newMO = startTimeP2.month() - 1;
            newDD = startTimeP2.day() - 1;
            newHH = 0;
            newMM = 0;

            currentModalState = MODAL_DURAT_P2_MM;    
            lcd.setCursor(0, 0);
            lcd.print("P2 DURAT MINUTES       ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_DURAT_P2_MM:

            currentModalState = MODAL_FREQ_P2_HH;
            lcd.setCursor(0, 0);
            lcd.print("P2 FREQ HOURS       ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
        break;
        case MODAL_FREQ_P2_HH:

            currentModalState = MODAL_RUN;
            lcd.setCursor(0, 0);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print("                ");            
            

        break;
        default:
            
            currentModalState = MODAL_SET_ONLY_PROG;            
            lcd.setCursor(0, 0);
            lcd.print("SET ONLY PROG?  ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");

            durationP1 = 0;
            durationP2 = 0;
            freqP1 = 0;
            freqP2 = 0;
            p2Enabled = 0;
            setOnlyProg = 0;
    }

    delay(BLINK_INT_MODAL_UPD);
    nextExecution = rtc.now() + TimeSpan(durationShowNext);
    
  } 

}


void switchToModalUpdYY(){

    currentModalState = MODAL_UPD_YY;            
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("SET YEAR       ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("YY");

    

}

void switchtoModalStartP1(){

    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("P1 START YEAR        ");             
    lcd.setCursor(0, 1);
    lcd.print("                ");                                     
    newYY = 2018 - 1;
    newMO = 0;
    newDD = 0;
    newHH = 0;
    newMM = 0;                               
    currentModalState = MODAL_START_P1_YY;    

}



void showSettingParams(){

    if(statusButtonSet == HIGH){              
                
        switch (currentModalState) {
            case MODAL_SET_ONLY_PROG:
                setOnlyProg = setOnlyProg ? 0 : 1;
                lcd.setCursor(1, 1);
                if(setOnlyProg){
                    lcd.print("YES        ");                        
                }else{
                    lcd.print("NO         ");                        
                }                
                break;      
            case MODAL_UPD_YY:                    
                newYY++;                                
                lcd.setCursor(3, 1);
                lcd.print(newYY);
                break;
            case MODAL_UPD_MO:                    
                newMO = newMO >= 12 ? 1 : ++newMO;                
                lcd.setCursor(3, 1);
                lcd.print(String(newMO > 9 ? String(newMO) : "0"+String(newMO)));
                break;
            case MODAL_UPD_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                lcd.setCursor(3, 1);
                lcd.print(String(newDD > 9 ? String(newDD) : "0"+String(newDD)));
                break;
            case MODAL_UPD_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                lcd.setCursor(3, 1);
                lcd.print(String(newHH > 9 ? String(newHH) : "0"+String(newHH)));
                break;
            case MODAL_UPD_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                lcd.setCursor(3, 1);
                lcd.print(String(newMM > 9 ? String(newMM) : "0"+String(newMM)));
                break;                            
            case MODAL_START_P1_YY:
                newYY++;                                
                lcd.setCursor(3, 1);
                lcd.print(newYY);
                break;             
            case MODAL_START_P1_MO:
                newMO = newMO >= 12 ? 1 : ++newMO;                
                lcd.setCursor(1, 1);
                lcd.print(String(newMO > 9 ? String(newMO) : "0"+String(newMO)));
                break;                            
            case MODAL_START_P1_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                lcd.setCursor(1, 1);
                lcd.print(String(newDD > 9 ? String(newDD) : "0"+String(newDD)));
                break;
            case MODAL_START_P1_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                lcd.setCursor(1, 1);
                lcd.print(String(newHH > 9 ? String(newHH) : "0"+String(newHH)));
                break;
            case MODAL_START_P1_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                lcd.setCursor(1, 1);
                lcd.print(String(newMM > 9 ? String(newMM) : "0"+String(newMM)));
                break;                                 
            case MODAL_DURAT_P1_MM:
                durationP1 = ++durationP1;                    
                lcd.setCursor(1, 1);
                lcd.print(String(durationP1 > 9 ? String(durationP1) : "0"+String(durationP1)));                    
                break;
            case MODAL_FREQ_P1_HH:

                freqP1 = ++freqP1;
                lcd.setCursor(1, 1);
                lcd.print(String(freqP1 > 9 ? String(freqP1) : "0"+String(freqP1)));                    
                break;
            case MODAL_P2_ENABLED:                
                p2Enabled = p2Enabled ? 0 : 1;
                lcd.setCursor(1, 1);
                if(p2Enabled){
                    lcd.print("YES        ");                        
                }else{
                    lcd.print("NO         ");                        
                }                
                break;                                                                
            case MODAL_START_P2_YY:
                newYY++;                                
                lcd.setCursor(3, 1);
                lcd.print(newYY);
                break;             
            case MODAL_START_P2_MO:
                newMO = newMO >= 12 ? 1 : ++newMO;                
                lcd.setCursor(1, 1);
                lcd.print(String(newMO > 9 ? String(newMO) : "0"+String(newMO)));
                break;                            
            case MODAL_START_P2_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                lcd.setCursor(1, 1);
                lcd.print(String(newDD > 9 ? String(newDD) : "0"+String(newDD)));
                break;
            case MODAL_START_P2_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                lcd.setCursor(1, 1);
                lcd.print(String(newHH > 9 ? String(newHH) : "0"+String(newHH)));
                break;
            case MODAL_START_P2_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                lcd.setCursor(1, 1);
                lcd.print(String(newMM > 9 ? String(newMM) : "0"+String(newMM)));
                break;               
            case MODAL_DURAT_P2_MM:
                durationP2 = ++durationP2;                    
                lcd.setCursor(1, 1);
                lcd.print(String(durationP2 > 9 ? String(durationP2) : "0"+String(durationP2)));                    
                break;
            case MODAL_FREQ_P2_HH:
                freqP2 = ++freqP2;
                lcd.setCursor(1, 1);
                lcd.print(String(freqP2 > 9 ? String(freqP2) : "0"+String(freqP2)));                    
                break;                
                

        }

        delay(BLINK_INT_MODAL_UPD);

    } 

}



void receiveGenericRfSignal(){

    if (rfReceiver.available()) {
    
        int value = rfReceiver.getReceivedValue();
        
        if (value == 0) {
            Serial.print("Unknown encoding");
        } else {
            Serial.print("Received ");
            Serial.print( rfReceiver.getReceivedValue() );
            Serial.print(" / ");
            Serial.print( rfReceiver.getReceivedBitlength() );
            Serial.print("bit ");
            Serial.print("Protocol: ");
            Serial.println( rfReceiver.getReceivedProtocol() );
        }        

        rfReceiver.resetAvailable();
  }


}

void receiveRfSignal(){

    if (rfReceiver.available()) {
    
        int value = rfReceiver.getReceivedValue();
        
        if (value == 0) {
            Serial.print("Unknown encoding");
            soilStatus = VAL_DRY;

        } else {
            Serial.print("Received ");
            Serial.print( rfReceiver.getReceivedValue() );
            Serial.print(" / ");
            Serial.print( rfReceiver.getReceivedBitlength() );
            Serial.print("bit ");
            Serial.print("Protocol: ");
            Serial.println( rfReceiver.getReceivedProtocol() );
        }

        value = map(rfReceiver.getReceivedValue(),VAL_WET_TRIGGER,VAL_MAX_WET,0,100);

        Serial.print("umidita_percent: ");
        Serial.print(value);

        if (value <= VAL_DRY){
            
            Serial.println(" Terreno asciutto");
            soilStatus = VAL_DRY;
        }

        if (value >= VAL_WET){
            Serial.println(" Terreno umido");
            soilStatus = VAL_WET;
        }

        rfReceiver.resetAvailable();
  }


}


void setIrrigation(){

    String strInfo = "";

    if(currentModalState <= MODAL_RUN && previousModalState != MODAL_DEFAULT_PREV_STATUS && currentModalState != previousModalState){
                
        strInfo = now.day() > 9 ? String(now.day()) : "0"+String(now.day());
        strInfo += "/";
        strInfo += now.month() > 9 ? String(now.month()) : "0"+String(now.month());
        strInfo += "/";
        strInfo += String(now.year());
        strInfo += " ";
        strInfo += now.hour() > 9 ? String(now.hour()) : "0"+String(now.hour());
        strInfo += ":";
        strInfo += now.minute() > 9 ? String(now.minute()) : "0"+String(now.minute());              
        strInfo += " Eletrov. SPENTA";
        Serial.println(strInfo);

        digitalWrite(RELAIS_EV_STOP, HIGH);        
        delay(VAL_IMPULSO_EV);
        digitalWrite(RELAIS_EV_STOP, LOW);
        previousModalState = currentModalState;

    }

    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2) && currentModalState != previousModalState){
        
        
        strInfo = now.day() > 9 ? String(now.day()) : "0"+String(now.day());
        strInfo += "/";
        strInfo += now.month() > 9 ? String(now.month()) : "0"+String(now.month());
        strInfo += "/";
        strInfo += String(now.year());
        strInfo += " ";
        strInfo += now.hour() > 9 ? String(now.hour()) : "0"+String(now.hour());
        strInfo += ":";
        strInfo += now.minute() > 9 ? String(now.minute()) : "0"+String(now.minute());              
        strInfo += " Eletrov. ACCESA";
        Serial.println(strInfo);

        digitalWrite(RELAIS_EV_START, HIGH);        
        delay(VAL_IMPULSO_EV);
        digitalWrite(RELAIS_EV_START, LOW);
        previousModalState = currentModalState;


    }    

}
