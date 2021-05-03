/*
 Based on M5Stack > TFT_Clock_Digital
*/

#include <M5StickCPlus.h>

#define TFT_GREY 0x5AEB

// Status
#define READY    0
#define RUN      1
#define CONTINUE 2
#define BREAK    3
#define SLEEP    4
#define STOP     9

uint8_t status = READY;

// Time
uint32_t targetTime = 0;  // for next 1 second timeout
int8_t mm, ss;   // minutes and seconds
byte oldmm, oldss;  // for display
float targetmm;           // for progress bar

int internalss = 0;       // for reminder beep
int reminderss = 5*60; // for reminder beep (senconds)

int8_t npom = 0;        // number of pomodoro

// Mode
int8_t mode = 0;        // Mode. -1) Demo
int8_t maxmode = 2; // max(mode). Check settime()

void settimer(int8_t md){
  if (md == 0) {
    mm = 25; ss = 0;
  } else if (md == 1) {
    mm = 48; ss = 0;
  } else if (md == 2) {
    mm = 60; ss = 0;
  } else if (md == -1) {
    mm =  0; ss = 15;
    //mm = 12; ss = 0;
  }
  targetmm = mm;
}

void setbreaktimer(int8_t md){
  if (md == 0) {
    mm =  5; ss = 0;
  } else if (md == 1) {
    mm = 12; ss = 0;
  } else if (md == 2) {
    mm = 15; ss = 0;
  } else if (md == -1) {
    mm =  0; ss = 7;
    //mm =  3; ss = 0;
  }
  targetmm = mm;
}

// Display setting
int8_t brightness = 8;
uint32_t colorbg, colortext;
  // M5StickC Plus 135x240
int xpos = 2, ypos = 34, xsecs = 0; // for numbers
int numsize = 8;                    // font size of numbers
int xpom = 16, ypom = 16, rpom = 8; // for pomodoro marks
int xbar =  0, ybar =125, hbar =10; // for progress bar
float wbar = 240.;


void displaytime(){
  // Update digital time
  if (oldmm != mm) { // Redraw minutes time every minute
    oldmm = mm;
    xpos = 0;
    // Draw hours and minutes
    if (mm < 10 && mm >=0) xpos += M5.Lcd.drawChar('0', xpos, ypos, numsize); // Add minutes leading zero
    xpos += M5.Lcd.drawNumber(mm, xpos, ypos, numsize);             // Draw minutes
    xsecs = xpos; // Save seconds 'x' position for later display updates
  }
  if (oldss != ss) { // Redraw seconds time every second
    oldss = ss;
    xpos = xsecs;
    xpos += M5.Lcd.drawChar(':', xsecs-6, ypos-8, numsize);
    //Draw seconds
    if (ss < 10) xpos += M5.Lcd.drawChar('0', xpos-8, ypos, numsize); // Add leading zero
    M5.Lcd.drawNumber(ss, xpos-10, ypos, numsize);                     // Draw seconds
  }
}

void displaybar(){
  if (targetmm != 0){
    if (oldmm != mm){
      M5.Lcd.fillRect(xbar, ybar, (int)(wbar - (float)(mm+1)*wbar/targetmm), hbar, colortext); 
    }
  }
}

void displayreset(uint8_t stts, int8_t np){
  oldmm = 99; oldss = 99;

  if (stts == READY) { 
    settimer(mode);
    colorbg = TFT_RED;
    colortext = TFT_WHITE;
  } else if (stts == CONTINUE) {
    colorbg = TFT_YELLOW;
    colortext = TFT_BLACK;    
  } else if (stts == BREAK) {
    setbreaktimer(mode);
    colorbg = TFT_DARKGREEN;
    colortext = TFT_WHITE;    
  }
  
  M5.Lcd.fillScreen(colorbg);
  M5.Lcd.setTextColor(colortext, colorbg);
  displaytime();
  int xxpom;
  for (int i=1; i<=np; i++){
    xxpom = xpom + (i-1) * (2*rpom + 4);
    M5.Lcd.fillCircle(xxpom, ypom, rpom, colortext); 
  }
}

void beep(){
  M5.Beep.setBeep(4000, 500);
  M5.Beep.beep();
  //LED
  digitalWrite(GPIO_NUM_10, LOW);
  delay(100);
  digitalWrite(GPIO_NUM_10, HIGH);
}

void reminder_beep(){
  internalss++;                         // Advance second
  if (internalss == reminderss+1) {  // Check for roll-over
    internalss = 0;
    M5.Beep.setBeep(4000, 100);
    M5.Beep.beep();
    delay(120);
    M5.Beep.update();  
    M5.Beep.beep();
  }  
}

//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////

void setup(void) {
  //Serial.begin(115200);
  M5.begin();
  M5.Lcd.setRotation(3); // 1) A is right 3) A is left
  M5.Lcd.setTextSize(1);
  M5.Axp.ScreenBreath(brightness);  // Display brightness
  setCpuFrequencyMhz(10);  // 10MHz

  pinMode(GPIO_NUM_10, OUTPUT);    // LED
  digitalWrite(GPIO_NUM_10, HIGH);

  displayreset(status, npom);
}

void loop() {
  M5.update();
  M5.Beep.update();  

  //** Button action ****************************************
  // A button => Status change
  if (M5.BtnA.wasPressed()) {
    if (status == READY || status == STOP) {
      status = RUN;    
      targetTime = millis() + 1000;
      M5.Lcd.drawCircle(xpom + npom * (2*rpom + 4), ypom, rpom, colortext);

    } else if (status == RUN) {
      status = STOP;    

    } else if (status == CONTINUE) {
      status = BREAK;
      displayreset(status, npom);
      targetTime = millis() + 1000;

    } else if (status == BREAK) {
      status = READY;
      displayreset(status, npom);
    }
  }

  // B button was pressed for a long time => Reset
  if (M5.BtnB.pressedFor(1000)) {
    status = READY;
    npom = 0;
    displayreset(status, npom);
  }

  // B button was pressed => Mode change
  if (M5.BtnB.wasPressed()) {
    if (status == READY) {
      if (mode == maxmode) {
        mode = -1; // 0 to remove demo
      } else {
        mode++;
      }
    } else {
      status = READY;
    }
    displayreset(status, npom);
  }

  // Power button was pressed => Sleep/Wake
  if (M5.Axp.GetBtnPress()==2) {
    if(status == READY) {
      status = SLEEP;
      //M5.Axp.SetSleep();
      M5.Axp.ScreenBreath(0); 
    }else if (status == SLEEP){
      status = READY;
      M5.Axp.ScreenBreath(brightness); 
    }
  }
  
  //** Timer action ****************************************
  if (targetTime < millis()) {
    // Set next update for 1 second later
    targetTime = millis() + 1000;
  
    if (status == RUN || status == BREAK) {     
        ss--;              // Advance second
        if (ss == -1) {    // Check for roll-over
          ss = 59;         // Reset seconds to 59
          mm--;            // Advance minute
        }
        displaybar();
        displaytime();
        
        if (mm == 0 && ss == 0){
          beep();
          if (status == RUN) {
            status = CONTINUE;
            npom++;
          } else if (status == BREAK) {
            status = READY;
          }
          displayreset(status, npom);
          internalss = 0;
        }

    } else if (status == CONTINUE) {
        ss++;              // Advance second
        if (ss == 60) {    // Check for roll-over
          ss = 0;          // Reset seconds to zero
          mm++;            // Advance minute
        }
        displaytime(); 
        reminder_beep();

    } else if (status == READY) {
      reminder_beep();
    }
  }
}
