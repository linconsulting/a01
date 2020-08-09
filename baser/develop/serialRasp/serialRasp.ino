#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"


RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#define DS3231_ADDRESS 0x68    // RTC is connected, address is Hex68 (Decimal 104)

#define BUTTON_SET 8
#define BUTTON_MOD 7
#define RELAIS_EV_STOP 9
#define RELAIS_EV_START 10
#define BLINK_INT_MODAL_UPD 500
#define MODAL_RUN 100
#define MODAL_IRRIG_ON_P1 110 
#define MODAL_IRRIG_ON_P2 120 
#define MODAL_IRRIG_ON_MANUAL 130 
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
#define MODAL_MENU_MANUAL_START 37
#define MODAL_MENU_MANUAL_STOP 38
#define MODAL_MENU_EXIT 40

#define VAL_IMPULSO_EV 200  


byte statusButtonMod = 0;
byte statusButtonSet = 0;
byte currentModalState = 0;
byte previousModalState = 0;
byte lcdStatus = 0;
byte switchModalMenu = 0;
byte numSkip = 0;

DateTime now = (uint32_t)0;
uint32_t unixNow;
uint32_t unixP1Start;
uint32_t unixP1Stop;
uint32_t unixP2Start;
uint32_t unixP2Stop;
DateTime startTimeP1 = (uint32_t)0;
DateTime startTimeP2 = (uint32_t)0;
DateTime lastExecution = (uint32_t)0;
DateTime nextExecution = (uint32_t)0;
byte durationP1 = 0;
byte durationP2 = 0;
byte durationShowNext = 15;
byte freqP1 = 0;
byte freqP2 = 0;
byte p2Enabled = 0;

int newYY = 2019;
byte newMO = 0;
byte newDD = 0;
byte newHH = 0;
byte newMM = 0;
byte newSS = 0;

byte soilStatus = 0; //0 terreno asciutto, 1 bagnato -- AL MOMENTO SEMPRE ASCIUTTO
byte debugMode = 0;


void setup() {

  pinMode(BUTTON_MOD, INPUT);  
  pinMode(BUTTON_SET, INPUT);    
  pinMode(RELAIS_EV_START, OUTPUT);
  pinMode(RELAIS_EV_STOP, OUTPUT);
  lcdOn();
  rtcSetup();
  currentModalState = MODAL_RUN;
  previousModalState = MODAL_DEFAULT_PREV_STATUS;
  
  if(debugMode){
      Serial.begin(9600);  
  }
  
  now = rtc.now();         
  nextExecution = now + TimeSpan(durationShowNext);
  setEvOff(); //chiudo ev x sicurezza: es in caso di avvenuto blackout durante irrigazione

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
    
    //receiveRfSignal();    
    ctrlTiming();
    ctrlIrrigationStatus();
    setIrrigation();

  }

}


void digitalClockDisplay(){

    //PREPARO STRINGA PRIMA RIGA DISPLAY

    byte day = now.day();                
    String strInfo = "";    
    if(day > 9){
        strInfo = String(day);
    }else{
        strInfo = "0"+String(day);
    }
    strInfo += "/";    
    
    byte month = now.month();
    if(month > 9){
        strInfo += String(month);
    }else{
        strInfo += "0"+String(month);
    }
    strInfo += "/";
    strInfo += String(now.year());    
    strInfo += " ";
    
    byte hour = now.hour();
    if(hour > 9){
        strInfo += String(hour);
    }else{
        strInfo += "0"+String(hour);
    }
    strInfo += ":";
    
    byte minute = now.minute();
    if(minute > 9){
        strInfo += String(minute);
    }else{
        strInfo += "0"+String(minute);
    }
    
    lcd.setCursor(0,0);
    lcd.print(strInfo);      

    //PREPARO STRINGA SECONDA RIGA DISPLAY      
    
    strInfo = "";

    uint32_t unixNextExecution = nextExecution.unixtime();

    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2)){
        
        //MENTRE AVVIENE L'IRRIGAZIONE
        strInfo += F(" IRRIGAZIONE ON ");                

    }else if(unixNow < unixNextExecution){

        //ULTIMA ESECUZIONE
        //U 01011970 00:00
        strInfo += "U ";                
        day = lastExecution.day();   
        if(day > 9){
            strInfo += String(day);
        }else{
            strInfo += "0"+String(day);
        }
                
        month = lastExecution.month();
        if(month > 9){
            strInfo += String(month);
        }else{
            strInfo += "0"+String(month);
        }
        strInfo += String(lastExecution.year());    
        strInfo += " ";

        hour = lastExecution.hour();
        if(hour > 9){
            strInfo += String(hour);
        }else{
            strInfo += "0"+String(hour);
        }
        strInfo += ":";
        minute = lastExecution.minute();
        if(minute > 9){
            strInfo += String(minute);
        }else{
            strInfo += "0"+String(minute);
        }

    }else{

        //PROSSIMA ESECUZIONE
        //P 01011970 00:00

        strInfo += "P ";

        if( p2Enabled == 0 || (unixP1Start < unixP2Start)){

            day = startTimeP1.day();   
            if(day > 9){
                strInfo += String(day);
            }else{
                strInfo += "0"+String(day);
            }
                    
            month = startTimeP1.month();
            if(month > 9){
                strInfo += String(month);
            }else{
                strInfo += "0"+String(month);
            }
            strInfo += String(startTimeP1.year());    
            strInfo += " ";            
            
            hour = startTimeP1.hour();
            if(hour > 9){
                strInfo += String(hour);
            }else{
                strInfo += "0"+String(hour);
            }
            strInfo += ":";
            minute = startTimeP1.minute();
            if(minute > 9){
                strInfo += String(minute);
            }else{
                strInfo += "0"+String(minute);
            }

        }else{
            
            day = startTimeP2.day();   
            if(day > 9){
                strInfo = String(day);
            }else{
                strInfo = "0"+String(day);
            }
                    
            month = startTimeP2.month();
            if(month > 9){
                strInfo += String(month);
            }else{
                strInfo += "0"+String(month);
            }
            strInfo += String(startTimeP2.year());    
            strInfo += " ";              
                        
            hour = startTimeP2.hour();
            if(hour > 9){
                strInfo += String(hour);
            }else{
                strInfo += "0"+String(hour);
            }
            strInfo += ":";
            minute = startTimeP2.minute();
            if(minute > 9){
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

    if(currentModalState == MODAL_IRRIG_ON_MANUAL){
        lastExecution = now;
        return;
    }

    //CK P1 -- START
    if(currentModalState >= MODAL_RUN && unixNow >= unixP1Start && soilStatus == 0){

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
    if(currentModalState >= MODAL_RUN && p2Enabled == 1 && unixNow >= unixP2Start && soilStatus == 0){
        
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
    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2) && soilStatus == 1){
        currentModalState = MODAL_RUN;
        lastExecution = now;
    }
    if(p2Enabled == 0 && currentModalState == MODAL_IRRIG_ON_P2){
        currentModalState = MODAL_RUN;
        startTimeP2 = (uint32_t)0;
    }
    

}

void switchToModalStartP1(){

    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("                "));
        lcd.setCursor(0, 0);
        lcd.print(F("P1 START YEAR        "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));                                     
    }
    
    newYY = now.year() - 1;
    newMO = now.month() - 1;
    newDD = now.day() - 1;
    newHH = now.hour() - 1;
    newMM = now.minute() - 1;                               
    currentModalState = MODAL_START_P1_YY;    

}



void rtcSetup(){

  if (! rtc.begin()) {
    if(lcdStatus){
        lcd.setCursor(0,0);
        lcd.print(F("Couldn't find RTC"));
    }
    while (1);
  }
  if (rtc.lostPower()) {

    if(lcdStatus){
        lcd.setCursor(0,0);
        lcd.print(F("RTC lost power,"));    
        lcd.setCursor(0,1);
        lcd.print(F("set the time!"));    
    }      
    
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

void resetBaseVars(){

    switchModalMenu = 0;                        
    durationP1 = 0;
    durationP2 = 0;
    freqP1 = 0;
    freqP2 = 0;
    p2Enabled = 0;                        
    numSkip = 0;
    nextExecution = (uint32_t)0;

}

void switchToModalMenu(){

    switch (switchModalMenu)
    {
        case MODAL_MENU_CLOCK:
            resetBaseVars();                        
            switchToModalUpdYY();
            break;

        case MODAL_MENU_PROGR:
            resetBaseVars();                        
            switchToModalStartP1();      
            break;

        case MODAL_MENU_PROGR_SKIP:
            switchModalMenu = 0;
            switchToModalSkip();      
            break;
        case MODAL_MENU_MANUAL_START:            
            switchModalMenu = 0;            
            switchToModalRun();
            setEvOn();                        
            currentModalState = MODAL_IRRIG_ON_MANUAL;
            break;
        case MODAL_MENU_MANUAL_STOP:            
            switchModalMenu = 0;
            setEvOff();
            switchToModalRun();
            break;

        case MODAL_MENU_EXIT:
            switchToModalRun();
            switchModalMenu = 0;
            break;
    
        default:
            switchToModalRun();
            switchModalMenu = 0;
            break;
    }

}

void switchToModalUpdMo(){    
    currentModalState = MODAL_UPD_MO;  
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("SET MONTH       "));
        lcd.setCursor(0, 1);
        lcd.print(F("MO        "));          
    }      

}

void switchToModalUpdDd(){
    currentModalState = MODAL_UPD_DD;    
    if(lcdStatus)
    {
        lcd.setCursor(0, 0);
        lcd.print(F("SET DAY       "));
        lcd.setCursor(0, 1);
        lcd.print(F("DD        "));   
    }     

}

void switchToModalUpdHh(){
    currentModalState = MODAL_UPD_HH;     
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("SET HOURS       "));
        lcd.setCursor(0, 1);
        lcd.print(F("HH        "));       
    }

}

void switchToModalUpdMm(){
    currentModalState = MODAL_UPD_MM;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("SET MINUTES       "));
        lcd.setCursor(0, 1);
        lcd.print(F("MM        "));        
    }

}

void switchToModalStartP1Mo(){

    currentModalState = MODAL_START_P1_MO;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P1 START MONTH        "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalStartP1Dd(){
    currentModalState = MODAL_START_P1_DD;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P1 START DAY        "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalStartP1Hh(){
    currentModalState = MODAL_START_P1_HH;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P1 START HOUR        "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalStartP1Mm(){
    currentModalState = MODAL_START_P1_MM;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P1 START MINUTES       "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalDuratP1Mm(){
    currentModalState = MODAL_DURAT_P1_MM;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P1 DURAT MINUTES       "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalFreqP1Hh(){
    currentModalState = MODAL_FREQ_P1_HH;
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P1 FREQ HOURS       "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalP2Enabled(){
    currentModalState = MODAL_P2_ENABLED;
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 ENABLED         "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }    

}

void switchToModalStartP2Yy(){
    currentModalState = MODAL_START_P2_YY;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 START YEAR        "));             
    }

}

void switchToModalRun(){    
    currentModalState = MODAL_RUN;
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("                "));
        lcd.setCursor(0, 1);
        lcd.print(F("                "));            
    }

}

void switchToModalStartP2Mo(){
    currentModalState = MODAL_START_P2_MO;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 START MONTH        "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}


void switchToModalStartP2Dd(){
    currentModalState = MODAL_START_P2_DD;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 START DAY        "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalStartP2Hh(){
    currentModalState = MODAL_START_P2_HH;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 START HOUR        "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalStartP2Mm(){
    currentModalState = MODAL_START_P2_MM;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 START MINUTES       "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalDuratP2Mm(){
    currentModalState = MODAL_DURAT_P2_MM;    
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 DURAT MINUTES       "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalFreqP2Hh(){
    currentModalState = MODAL_FREQ_P2_HH;
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("P2 FREQ HOURS       "));             
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

void switchToModalMenuEnter(){
    currentModalState = MODAL_MENU_ENTER;
    switchModalMenu = 0;
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("IMPOSTA         "));            
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
    }

}

/*
* setStatusModal() e showSettingParams() lavorano
* in sincronia: la prima è associata alla pressione del
* pulsante MOD, la seconda la pulsante SET.
*
* SI INIZIA QUANDO SI PREME IL PULSANTE MOD    
* inizialmente currentModalState = MODAL_RUN
* lo switch parte dal default che lo imposta = MODAL_MENU_ENTER
* essendo MODAL_MENU_ENTER < MODAL_RUN
* poi la funzione loop chiama showSettingParams()
* per la modifica delle relative voci.
*
*/
void setStatusModal(){
  
  if(statusButtonMod == HIGH){

    switch (currentModalState) {

        case MODAL_MENU_ENTER:
            switchToModalMenu();            
        break;
        case MODAL_MENU_PROGR_SKIP:            
            switchToModalRun();            
        break;        
        case MODAL_UPD_YY:
            switchToModalUpdMo();            
        break;    
        case MODAL_UPD_MO:        
            switchToModalUpdDd();                        
        break;
        case MODAL_UPD_DD:        
            switchToModalUpdHh();                        
        break;
        case MODAL_UPD_HH:        
            switchToModalUpdMm();                        
        break;
        case MODAL_UPD_MM:   

            rtc.adjust(DateTime(newYY, newMO, newDD, newHH, newMM, newSS));
            switchToModalStartP1();

        break;    
        case MODAL_START_P1_YY:
            switchToModalStartP1Mo();
            
        break;
        case MODAL_START_P1_MO:
            switchToModalStartP1Dd();           
            
        break;
        case MODAL_START_P1_DD:
            switchToModalStartP1Hh();       
            
        break;
        case MODAL_START_P1_HH:
            switchToModalStartP1Mm();      
                        
        break;
        case MODAL_START_P1_MM:

            startTimeP1 = DateTime(newYY, newMO, newDD, newHH, newMM, newSS);
            newYY = now.year() - 1;
            newMO = now.month() - 1;
            newDD = now.day() - 1;
            newHH = now.hour() - 1;
            newMM = now.minute() -1;

            switchToModalDuratP1Mm();
            
        break;        
        case MODAL_DURAT_P1_MM:
            switchToModalFreqP1Hh();            
        break;
        case MODAL_FREQ_P1_HH:
            switchToModalP2Enabled();
        break;

        case MODAL_P2_ENABLED:

            if(p2Enabled == 0){
                switchToModalRun();                      
                
            }else{
                switchToModalStartP2Yy();      
            }            
            if(lcdStatus){
                lcd.setCursor(0, 1);
                lcd.print(F("                "));
            }
            
        break;

        case MODAL_START_P2_YY:
            switchToModalStartP2Mo();            
            
        break;
        case MODAL_START_P2_MO:
            switchToModalStartP2Dd();            
            
        break;
        case MODAL_START_P2_DD:
            switchToModalStartP2Hh();
            
        break;
        case MODAL_START_P2_HH:
            switchToModalStartP2Mm();            
            
        break;
        case MODAL_START_P2_MM:

            startTimeP2 = DateTime(newYY, newMO, newDD, newHH, newMM, newSS);
            newYY = now.year() - 1;
            newMO = now.month() - 1;
            newDD = now.day() - 1;
            newHH = now.hour() -1;
            newMM = now.minute() -1;

            switchToModalDuratP2Mm();
            
        break;
        case MODAL_DURAT_P2_MM:
            switchToModalFreqP2Hh();            
            
        break;
        case MODAL_FREQ_P2_HH:
            switchToModalRun();            

        break;
        default:
            switchToModalMenuEnter();
            
    }

    delay(BLINK_INT_MODAL_UPD);
    nextExecution = rtc.now() + TimeSpan(durationShowNext);
    
  } 

}


void switchToModalUpdYY(){

    currentModalState = MODAL_UPD_YY;            
    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("                "));
        lcd.setCursor(0, 0);
        lcd.print(F("SET YEAR        "));
        lcd.setCursor(0, 1);
        lcd.print(F("                "));
        lcd.setCursor(0, 1);
        lcd.print(F("YY"));
    }
       

}


void switchToModalSkip(){

    if(lcdStatus){
        lcd.setCursor(0, 0);
        lcd.print(F("                "));
        lcd.setCursor(0, 0);
        lcd.print(F("N. SKIP         "));             
        lcd.setCursor(0, 1);
        lcd.print(F("0               "));                                     
    }    
    numSkip = 0;
    currentModalState = MODAL_MENU_PROGR_SKIP;
}

/* 
* QUANDO SI PREME IL BOTTONE SET
* in base alla combinazione dei valori di currentModalState e switchModalMenu
* si ha la possibilità di compiere le azioni.
* Esempio quando currentModalState=MODAL_MENU_ENTER e switchModalMenu<>0
* ad ogni pressione del bottone set si passa alla voce di menù successivo.
* Nello switch si inizia dal MODAL_MENU_ENTER poi dal suo default.
*/

void showSettingParams(){

    if(statusButtonSet == HIGH){              
                
        switch (currentModalState) {

            case MODAL_MENU_ENTER:

                switch (switchModalMenu)
                {
                    case MODAL_MENU_CLOCK:
                        if(lcdStatus){
                            lcd.setCursor(1, 1);
                            lcd.print(F("PROGRAMMI       "));                        
                        }                        
                        switchModalMenu = MODAL_MENU_PROGR;
                        break;
                    case MODAL_MENU_PROGR:
                        if(lcdStatus){
                            lcd.setCursor(1, 1);
                            lcd.print(F("SKIP PROGRAMMI  "));         
                        }                        
                        switchModalMenu = MODAL_MENU_PROGR_SKIP;
                        break;
                    case MODAL_MENU_PROGR_SKIP:
                        if(lcdStatus){
                            lcd.setCursor(1, 1);
                            lcd.print(F("START MANUALE   "));         
                        }                        
                        switchModalMenu = MODAL_MENU_MANUAL_START;
                        break;
                    case MODAL_MENU_MANUAL_START:
                        if(lcdStatus){
                            lcd.setCursor(1, 1);
                            lcd.print(F("STOP MANUALE    "));         
                        }                        
                        switchModalMenu = MODAL_MENU_MANUAL_STOP;
                        break;                        
                    case MODAL_MENU_MANUAL_STOP:
                        if(lcdStatus){
                            lcd.setCursor(1, 1);
                            lcd.print(F("ESCI            "));    
                        }                        
                        switchModalMenu = MODAL_MENU_EXIT;               
                        break;
                    case MODAL_MENU_EXIT:
                        if(lcdStatus){
                            lcd.setCursor(1, 1);
                            lcd.print(F("OROLOGIO        "));     
                        }
                        switchModalMenu = MODAL_MENU_CLOCK;                        
                        break;
                
                    default:
                        if(lcdStatus){
                            lcd.setCursor(1, 1);
                            lcd.print(F("OROLOGIO        "));                                                
                        }                        
                        switchModalMenu = MODAL_MENU_CLOCK;                        
                        break;
                }
                break;

            case MODAL_MENU_PROGR_SKIP:
                numSkip++;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    lcd.print(numSkip);
                }                
                break;
            case MODAL_UPD_YY:                    
                newYY++;                                
                if(lcdStatus){
                    lcd.setCursor(3, 1);
                    lcd.print(newYY);
                }                
                break;
            case MODAL_UPD_MO:                    
                newMO = newMO >= 12 ? 1 : ++newMO;                
                if(lcdStatus){
                    lcd.setCursor(3, 1);
                    if(newMO <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newMO);
                }                
                break;
            case MODAL_UPD_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                if(lcdStatus){
                    lcd.setCursor(3, 1);
                    if(newDD <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newDD);
                }                
                break;
            case MODAL_UPD_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                if(lcdStatus){
                    lcd.setCursor(3, 1);
                    if(newHH <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newHH);   
                }                             
                break;
            case MODAL_UPD_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                if(lcdStatus){
                    lcd.setCursor(3, 1);
                    if(newMM <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newMM); 
                }                
                break;                            
            case MODAL_START_P1_YY:
                newYY++;                                
                if(lcdStatus){
                    lcd.setCursor(3, 1);
                    lcd.print(newYY);
                }
                
                break;             
            case MODAL_START_P1_MO:
                newMO = newMO >= 12 ? 1 : ++newMO;                
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newMO <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newMO);
                }
                
                break;                            
            case MODAL_START_P1_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newDD <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newDD);
                }
                
                break;
            case MODAL_START_P1_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newHH <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newHH); 
                }
                
                break;
            case MODAL_START_P1_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newMM <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newMM); 
                }                
                break;                                 
            case MODAL_DURAT_P1_MM:
                durationP1 = ++durationP1;                    
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(durationP1 <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(durationP1);
                }                
                break;
            case MODAL_FREQ_P1_HH:
                freqP1 = ++freqP1;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(freqP1 <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(freqP1);
                }                
                break;
            case MODAL_P2_ENABLED:                
                p2Enabled = p2Enabled ? 0 : 1;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(p2Enabled){
                        lcd.print(F("YES        "));                        
                    }else{
                        lcd.print(F("NO         "));                        
                    }                
                }                
                break;                                                                
            case MODAL_START_P2_YY:
                newYY++;             
                if(lcdStatus){
                    lcd.setCursor(3, 1);
                    lcd.print(newYY);
                }
                break;             
            case MODAL_START_P2_MO:
                newMO = newMO >= 12 ? 1 : ++newMO;                
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newMO <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newMO);
                }                
                break;                            
            case MODAL_START_P2_DD:                    
                newDD = newDD >= 31 ? 1 : ++newDD;                
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newDD <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newDD);
                }                
                break;
            case MODAL_START_P2_HH:                    
                newHH = newHH >= 24 ? 1 : ++newHH;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newHH <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newHH); 
                }                
                break;
            case MODAL_START_P2_MM:                    
                newMM = newMM >= 60 ? 1 : ++newMM;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(newMM <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(newMM);
                }                
                break;               
            case MODAL_DURAT_P2_MM:
                durationP2 = ++durationP2;       
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(durationP2 <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(durationP2);
                }                             
                break;
            case MODAL_FREQ_P2_HH:
                freqP2 = ++freqP2;
                if(lcdStatus){
                    lcd.setCursor(1, 1);
                    if(freqP2 <= 9){
                        lcd.print(0);                    
                    }
                    lcd.print(freqP2);
                }                
                break;                                

        }

        delay(BLINK_INT_MODAL_UPD);

    } 

}



void setIrrigation(){

    if(currentModalState == MODAL_IRRIG_ON_MANUAL){
        return;
    }

    String strInfo = "";

    if(currentModalState <= MODAL_RUN && previousModalState != MODAL_DEFAULT_PREV_STATUS && currentModalState != previousModalState){
                
        if(debugMode){

            byte day = now.day();                
            if(day > 9){
                strInfo = String(day);
            }else{
                strInfo = "0"+String(day);
            }
            strInfo += "/";
            byte month = now.month();
            if(month > 9){
                strInfo += String(month);
            }else{
                strInfo += "0"+String(month);
            }
            strInfo += "/";
            strInfo += String(now.year());    
            strInfo += " ";
            byte hour = now.hour();
            if(hour > 9){
                strInfo += String(hour);
            }else{
                strInfo += "0"+String(hour);
            }
            strInfo += ":";
            byte minute = now.minute();
            if(minute > 9){
                strInfo += String(minute);
            }else{
                strInfo += "0"+String(minute);
            }
            strInfo += " Eletrov. SPENTA";

            Serial.println(strInfo);
        }

        setEvOff();
        previousModalState = currentModalState;

    }

    if((currentModalState == MODAL_IRRIG_ON_P1 || currentModalState == MODAL_IRRIG_ON_P2) && currentModalState != previousModalState){
        
        
        if(debugMode){

            byte day = now.day();                
            if(day > 9){
                strInfo = String(day);
            }else{
                strInfo = "0"+String(day);
            }
            strInfo += "/";
            byte month = now.month();
            if(month > 9){
                strInfo += String(month);
            }else{
                strInfo += "0"+String(month);
            }
            strInfo += "/";
            strInfo += String(now.year());    
            strInfo += " ";
            byte hour = now.hour();
            if(hour > 9){
                strInfo += String(hour);
            }else{
                strInfo += "0"+String(hour);
            }
            strInfo += ":";
            byte minute = now.minute();
            if(minute > 9){
                strInfo += String(minute);
            }else{
                strInfo += "0"+String(minute);
            }           
            strInfo += " Eletrov. ACCESA";           
            
            Serial.println(strInfo);
        }

        setEvOn();
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

//apre elettrovalvola
void setEvOn(){
    digitalWrite(RELAIS_EV_START, HIGH);        
    delay(VAL_IMPULSO_EV);
    digitalWrite(RELAIS_EV_START, LOW);

}
//chiude elettrovalvola
void setEvOff(){
    digitalWrite(RELAIS_EV_STOP, HIGH);        
    delay(VAL_IMPULSO_EV);
    digitalWrite(RELAIS_EV_STOP, LOW);

}
