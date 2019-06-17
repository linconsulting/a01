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

#define MODAL_MENU_ENTER 30
#define MODAL_MENU_CLOCK 32
#define MODAL_MENU_PROGR 34
#define MODAL_MENU_PROGR_SKIP 36
#define MODAL_MENU_EXIT 40


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
byte switchModalMenu = 0;
byte numSkip = 0;

DateTime now = 0;
uint32_t unixNow;
uint32_t unixP1Start;
uint32_t unixP1Stop;
uint32_t unixP2Start;
uint32_t unixP2Stop;
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
  setSafeEV(); //spengo ev in caso di avvenuto blackout durante irrigazione

}

void loop() {
   
  
  statusButtonMod = digitalRead(BUTTON_MOD);
  statusButtonSet = digitalRead(BUTTON_SET);

  setStatusModal();      //QUANDO SI PREME IL BOTTONE MOD
  

  if(currentModalState < MODAL_RUN){
    
    showSettingParams();   //QUANDO SI PREME IL BOTTONE SET
    
    
  }else{

    if(statusButtonSet == HIGH){
        if(lcdStatus == 1){
            lcdOff();
        }else{
            lcdOn();
        }
        delay(2000);
    }

    now = rtc.now();
    unixNow = now.unixtime();
        
    if(lcdStatus == 1){
        digitalClockDisplay();    
    }
    
    receiveRfSignal();    
    ctrlTiming();
    ctrlIrrigationStatus();
    setIrrigation();

  }

}


void digitalClockDisplay(){

    byte day = now.day();                
    String strInfo = "";    
    if(day < 9){
        strInfo = String(day);
    }else{
        strInfo = "0"+String(day);
    }
    strInfo += "/";    
    byte month = now.month();
    if(month < 9){
        strInfo += String(month);
    }else{
        strInfo += "0"+String(month);
    }
    strInfo += "/";
    strInfo += String(now.year());    
    strInfo += " ";
    byte hour = now.hour();
    if(hour < 9){
        strInfo += String(hour);
    }else{
        strInfo += "0"+String(hour);
    }
    strInfo += ":";
    byte minute = now.minute();
    if(minute < 9){
        strInfo += String(minute);
    }else{
        strInfo += "0"+String(minute);
    }
    
    lcd.setCursor(0,0);
    lcd.print(strInfo);            
    
    strInfo = soilStatus == VAL_DRY ? "TA" : "TU";

    uint32_t unixNextExecution = nextExecution.unixtime();

    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2)){

        strInfo += " - UE ";        
        if(hour < 9){
            strInfo += String(hour);
        }else{
            strInfo += "0"+String(hour);
        }
        strInfo += ":";
        if(minute < 9){
            strInfo += String(minute);
        }else{
            strInfo += "0"+String(minute);
        }

    }else if(unixNow < unixNextExecution){

        strInfo += " - UE ";                
        hour = lastExecution.hour();
        if(hour < 9){
            strInfo += String(hour);
        }else{
            strInfo += "0"+String(hour);
        }
        strInfo += ":";
        minute = lastExecution.minute();
        if(minute < 9){
            strInfo += String(minute);
        }else{
            strInfo += "0"+String(minute);
        }

    }else{

        strInfo += " - PE ";

        if( p2Enabled == 0 || (unixP1Start < unixP2Start)){
            hour = startTimeP1.hour();
            if(hour < 9){
                strInfo += String(hour);
            }else{
                strInfo += "0"+String(hour);
            }
            strInfo += ":";
            minute = startTimeP1.minute();
            if(minute < 9){
                strInfo += String(minute);
            }else{
                strInfo += "0"+String(minute);
            }

        }else{
            hour = startTimeP2.hour();
            if(hour < 9){
                strInfo += String(hour);
            }else{
                strInfo += "0"+String(hour);
            }
            strInfo += ":";
            minute = startTimeP2.minute();
            if(minute < 9){
                strInfo += String(minute);
            }else{
                strInfo += "0"+String(minute);
            }
        }
       
        if(unixNow >= (unixNextExecution + durationShowNext)){
            nextExecution = now + TimeSpan(durationShowNext);
        }

    }
    
    
    lcd.setCursor(0,1);
    lcd.print(strInfo);
    
}


void ctrlIrrigationStatus(){

    //CK P1 -- START
    if(currentModalState >= MODAL_RUN && unixNow >= unixP1Start && soilStatus == VAL_DRY){

        if(numSkip > 0){
            numSkip--;
            nextTime(1);            
        }else{
            currentModalState = MODAL_IRRIG_ON_P1;
        }
        
    }
    
    //CK P1 -- END
    if(currentModalState == MODAL_IRRIG_ON_P1 && unixNow >= unixP1Stop){
        currentModalState = MODAL_RUN;
        nextTime(1);        
        lastExecution = now;
    }

        
    //CK P2 -- START
    if(currentModalState >= MODAL_RUN && p2Enabled == 1 && unixNow >= unixP2Start && soilStatus == VAL_DRY){
        
        if(numSkip > 0){
            numSkip--;
            nextTime(2);
        }else{
            currentModalState = MODAL_IRRIG_ON_P2;
        }

    }

    //CK P2 -- END
    if(p2Enabled == 1 && currentModalState == MODAL_IRRIG_ON_P2 && unixNow >= unixP2Stop){
        currentModalState = MODAL_RUN;
        nextTime(2);
        lastExecution = now;
    }
    
    //CK FOR STOP IRRIGATION
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

        case MODAL_MENU_ENTER:

                switch (switchModalMenu)
                {
                    case MODAL_MENU_CLOCK:
                        switchModalMenu = 0;                        
                        durationP1 = 0;
                        durationP2 = 0;
                        freqP1 = 0;
                        freqP2 = 0;
                        p2Enabled = 0;                        
                        numSkip = 0;
                        nextExecution = 0;
                        switchToModalUpdYY();
                        break;

                    case MODAL_MENU_PROGR:
                        switchModalMenu = 0;                        
                        durationP1 = 0;
                        durationP2 = 0;
                        freqP1 = 0;
                        freqP2 = 0;
                        p2Enabled = 0;                        
                        numSkip = 0;
                        nextExecution = 0;
                        switchtoModalStartP1();      
                        break;

                    case MODAL_MENU_PROGR_SKIP:
                        switchModalMenu = 0;
                        switchtoModalSkip();      
                        break;

                    case MODAL_MENU_EXIT:
                        currentModalState = MODAL_RUN;
                        switchModalMenu = 0;
                        break;
                
                    default:
                        currentModalState = MODAL_RUN;
                        switchModalMenu = 0;
                        break;
                }


        break;
        case MODAL_MENU_PROGR_SKIP:
            currentModalState = MODAL_RUN;     
            lcd.setCursor(0, 0);
            lcd.print("                ");             
            lcd.setCursor(0, 1);
            lcd.print("                ");
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
            newHH = -1;
            newMM = -1;

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
            newHH = -1;
            newMM = -1;

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
            
            currentModalState = MODAL_MENU_ENTER;
            switchModalMenu = 0;
            lcd.setCursor(0, 0);
            lcd.print("IMPOSTA         ");            
            lcd.setCursor(0, 1);
            lcd.print("                ");
            
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
    lcd.print("SET YEAR        ");
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
    newYY = 2019 - 1;
    newMO = 0;
    newDD = 0;
    newHH = -1;
    newMM = -1;                               
    currentModalState = MODAL_START_P1_YY;    

}

void switchtoModalSkip(){
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("N. SKIP         ");             
    lcd.setCursor(0, 1);
    lcd.print("                ");                                     
    numSkip = 0;
    currentModalState = MODAL_MENU_PROGR_SKIP;
}



void showSettingParams(){

    if(statusButtonSet == HIGH){              
                
        switch (currentModalState) {

            case MODAL_MENU_ENTER:

                switch (switchModalMenu)
                {
                    case MODAL_MENU_CLOCK:
                        lcd.setCursor(1, 1);
                        lcd.print("PROGRAMMI       ");                        
                        switchModalMenu = MODAL_MENU_PROGR;
                        break;
                    case MODAL_MENU_PROGR:
                        lcd.setCursor(1, 1);
                        lcd.print("SKIP PROGRAMMI  ");         
                        switchModalMenu = MODAL_MENU_PROGR_SKIP;
                        break;
                    case MODAL_MENU_PROGR_SKIP:
                        lcd.setCursor(1, 1);
                        lcd.print("ESCI            ");    
                        switchModalMenu = MODAL_MENU_EXIT;               
                        break;
                    case MODAL_MENU_EXIT:
                        switchModalMenu = MODAL_MENU_CLOCK;
                        lcd.setCursor(1, 1);
                        lcd.print("OROLOGIO        ");     
                        break;
                
                    default:
                        switchModalMenu = MODAL_MENU_CLOCK;
                        lcd.setCursor(1, 1);
                        lcd.print("OROLOGIO        ");                                                
                        break;
                }
                break;

            case MODAL_MENU_PROGR_SKIP:
                numSkip++;
                lcd.setCursor(1, 1);
                lcd.print(numSkip);
                break;
            case MODAL_UPD_YY:                    
                newYY++;                                
                lcd.setCursor(3, 1);
                lcd.print(newYY);
                break;
            case MODAL_UPD_MO:                    
                newMO = newMO >= 12 ? 1 : ++newMO;                
                lcd.setCursor(3, 1);
                if(newMO <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newMO);
                break;
            case MODAL_UPD_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                lcd.setCursor(3, 1);
                if(newDD <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newDD);
                break;
            case MODAL_UPD_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                lcd.setCursor(3, 1);
                if(newHH <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newHH);                
                break;
            case MODAL_UPD_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                lcd.setCursor(3, 1);
                if(newMM <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newMM); 
                break;                            
            case MODAL_START_P1_YY:
                newYY++;                                
                lcd.setCursor(3, 1);
                lcd.print(newYY);
                break;             
            case MODAL_START_P1_MO:
                newMO = newMO >= 12 ? 1 : ++newMO;                
                lcd.setCursor(1, 1);
                if(newMO <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newMO);
                break;                            
            case MODAL_START_P1_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                lcd.setCursor(1, 1);
                if(newDD <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newDD);
                break;
            case MODAL_START_P1_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                lcd.setCursor(1, 1);
                if(newHH <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newHH); 
                break;
            case MODAL_START_P1_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                lcd.setCursor(1, 1);
                if(newMM <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newMM); 
                break;                                 
            case MODAL_DURAT_P1_MM:
                durationP1 = ++durationP1;                    
                lcd.setCursor(1, 1);
                if(durationP1 <= 9){
                    lcd.print(0);                    
                }
                lcd.print(durationP1);
                break;
            case MODAL_FREQ_P1_HH:
                freqP1 = ++freqP1;
                lcd.setCursor(1, 1);
                if(freqP1 <= 9){
                    lcd.print(0);                    
                }
                lcd.print(freqP1);
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
                if(newMO <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newMO);
                break;                            
            case MODAL_START_P2_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                lcd.setCursor(1, 1);
                if(newDD <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newDD);
                break;
            case MODAL_START_P2_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                lcd.setCursor(1, 1);
                if(newHH <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newHH); 
                break;
            case MODAL_START_P2_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                lcd.setCursor(1, 1);
                if(newMM <= 9){
                    lcd.print(0);                    
                }
                lcd.print(newMM);
                break;               
            case MODAL_DURAT_P2_MM:
                durationP2 = ++durationP2;                    
                lcd.setCursor(1, 1);
                if(durationP2 <= 9){
                    lcd.print(0);                    
                }
                lcd.print(durationP2);
                break;
            case MODAL_FREQ_P2_HH:
                freqP2 = ++freqP2;
                lcd.setCursor(1, 1);
                if(freqP2 <= 9){
                    lcd.print(0);                    
                }
                lcd.print(freqP2);
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
                
        byte day = now.day();                
        if(day < 9){
            strInfo = String(day);
        }else{
            strInfo = "0"+String(day);
        }
        strInfo += "/";
        byte month = now.month();
        if(month < 9){
            strInfo += String(month);
        }else{
            strInfo += "0"+String(month);
        }
        strInfo += "/";
        strInfo += String(now.year());    
        strInfo += " ";
        byte hour = now.hour();
        if(hour < 9){
            strInfo += String(hour);
        }else{
            strInfo += "0"+String(hour);
        }
        strInfo += ":";
        byte minute = now.minute();
        if(minute < 9){
            strInfo += String(minute);
        }else{
            strInfo += "0"+String(minute);
        }
        strInfo += " Eletrov. SPENTA";
        Serial.println(strInfo);

        digitalWrite(RELAIS_EV_STOP, HIGH);        
        delay(VAL_IMPULSO_EV);
        digitalWrite(RELAIS_EV_STOP, LOW);
        previousModalState = currentModalState;

    }

    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2) && currentModalState != previousModalState){
        
        
        byte day = now.day();                
        if(day < 9){
            strInfo = String(day);
        }else{
            strInfo = "0"+String(day);
        }
        strInfo += "/";
        byte month = now.month();
        if(month < 9){
            strInfo += String(month);
        }else{
            strInfo += "0"+String(month);
        }
        strInfo += "/";
        strInfo += String(now.year());    
        strInfo += " ";
        byte hour = now.hour();
        if(hour < 9){
            strInfo += String(hour);
        }else{
            strInfo += "0"+String(hour);
        }
        strInfo += ":";
        byte minute = now.minute();
        if(minute < 9){
            strInfo += String(minute);
        }else{
            strInfo += "0"+String(minute);
        }           
        strInfo += " Eletrov. ACCESA";
        Serial.println(strInfo);

        digitalWrite(RELAIS_EV_START, HIGH);        
        delay(VAL_IMPULSO_EV);
        digitalWrite(RELAIS_EV_START, LOW);
        previousModalState = currentModalState;


    }    

}


void ctrlTiming(){

    unixP1Start = startTimeP1.unixtime();
    unixP1Stop = unixP1Start + (durationP1*60);

    if(p2Enabled == 1){
        unixP2Start = startTimeP2.unixtime();
        unixP2Stop = unixP2Start + (durationP2*60);    
    }else{
        unixP2Start = 0;
        unixP2Stop = 0;
    }

}

void nextTime(int pn){

    if(pn == 1){
        startTimeP1 = startTimeP1 + TimeSpan(freqP1*3600UL);    
    }

    if(pn == 2){
        startTimeP2 = startTimeP2 + TimeSpan(freqP2*3600UL);
    }
   

}

void setSafeEV(){
    digitalWrite(RELAIS_EV_STOP, HIGH);        
    delay(VAL_IMPULSO_EV);
    digitalWrite(RELAIS_EV_STOP, LOW);
}
