/*
 Based on M5Stack > TFT_Clock_Digital

*/

#include <M5StickCPlus.h>

#define TFT_GREY 0x5AEB

#define READY 0
#define RUN 1
#define CONTINUE 2
#define BREAK 3
#define STOP 9

uint32_t targetTime = 0;  // for next 1 second timeout
int8_t mm = 25, ss = 0;
float pmm = 25;
byte omm = 99, oss = 99;

uint32_t colorbg, colortext;

int8_t pommode = 0;
uint8_t s = READY;
int8_t npom = 0;

int xpos = 2, ypos = 34, xsecs = 0;
int numsize = 8;
int xpom = 16, ypom = 16, rpom = 8, xxpom;
int xbar =  0, ybar =125, hbar = 10; // M5StickC Plus 135x240
float wbar = 240.;

void settimer(int8_t p){
  if (p == 0) {
    mm = 25; ss = 0;
  } else if (p == 1) {
    mm = 48; ss = 0;
  } else if (p == 2) {
    mm = 60; ss = 0;
  } else if (p == -1) {
    mm =  0; ss = 15;
  }
  pmm = mm;
}

void setbreaktimer(int8_t p){
  if (p == 0) {
    mm =  5; ss = 0;
  } else if (p == 1) {
    mm = 12; ss = 0;
  } else if (p == 2) {
    mm = 15; ss = 0;
  } else if (p == -1) {
    mm =  0; ss = 7;
  }
  pmm = mm;
}

void displaytime(){
  // Update digital time
  if (omm != mm) { // Redraw minutes time every minute
    omm = mm;
    xpos = 0;
    // Draw hours and minutes
    if (mm < 10 && mm >=0) xpos += M5.Lcd.drawChar('0', xpos, ypos, numsize); // Add minutes leading zero
    xpos += M5.Lcd.drawNumber(mm, xpos, ypos, numsize);             // Draw minutes
    xsecs = xpos; // Save seconds 'x' position for later display updates
  }
  if (oss != ss) { // Redraw seconds time every second
    oss = ss;
    xpos = xsecs;
    xpos += M5.Lcd.drawChar(':', xsecs-6, ypos-8, numsize);
    //Draw seconds
    if (ss < 10) xpos += M5.Lcd.drawChar('0', xpos-8, ypos, numsize); // Add leading zero
    M5.Lcd.drawNumber(ss, xpos-10, ypos, numsize);                     // Draw seconds
  }
}

void displaybar(){
  if (omm != mm){
    M5.Lcd.fillRect(xbar, ybar, (int)(wbar - (float)mm*wbar/(pmm+1.)), hbar, colortext); 
  }
}

void displayreset(){
  omm = 99; oss = 99;

  if (s == READY) { 
    settimer(pommode);
    colorbg = TFT_RED;
    colortext = TFT_WHITE;
  } else if (s == CONTINUE) {
    colorbg = TFT_YELLOW;
    colortext = TFT_BLACK;    
  } else if (s == BREAK) {
    setbreaktimer(pommode);
    colorbg = TFT_DARKGREEN;
    colortext = TFT_WHITE;    
  }
  
  M5.Lcd.fillScreen(colorbg);
  M5.Lcd.setTextColor(colortext, colorbg);
  displaytime();
  for (int i=1; i<=npom; i++){
    xxpom = xpom + (i-1) * (2*rpom + 4);
    M5.Lcd.fillCircle(xxpom, ypom, rpom, colortext); 
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
  M5.Beep.setBeep(4000, 500);

  M5.Axp.ScreenBreath(8);  // Display brightness
  setCpuFrequencyMhz(10);  // 10MHz
  
  pommode = 0; // -1) Demo
  npom = 0;
  s = READY;
  displayreset();
}

void loop() {
  M5.update();
  M5.Beep.update();  
  if (M5.BtnA.wasPressed()) {
    if (s == READY || s == STOP) {
      s = RUN;    
      targetTime = millis() + 1000;
      M5.Lcd.drawCircle(xpom + npom * (2*rpom + 4), ypom, rpom, TFT_WHITE);
    } else if (s == RUN) {
      s = STOP;    
    } else if (s == CONTINUE) {
      s = BREAK;
      displayreset();
      targetTime = millis() + 1000;
    } else if (s == BREAK) {
      s = READY;
      displayreset();
    }
  }
  if (M5.BtnB.pressedFor(1000)) {
    s = READY;
    npom = 0;
    displayreset();
  }
  if (M5.BtnB.wasPressed()) {
    if (s == READY) {
      if (pommode == 2) {
        pommode = -1; // 0 to remove demo
      } else {
        pommode++;
      }
    } else {
      s = READY;
    }
    displayreset();
  }

  if (s == RUN || s == BREAK) {  
    if (targetTime < millis()) {
      // Set next update for 1 second later
      targetTime = millis() + 1000;
  
      // Adjust the time values by adding 1 second
      ss--;              // Advance second
      if (ss == -1) {    // Check for roll-over
        ss = 59;         // Reset seconds to zero
        omm = mm;        // Save last minute time for display update
        mm--;            // Advance minute
      }
      displaybar();
      displaytime();
      
      if (mm == 0 && ss == 0){
        M5.Beep.beep();
        if (s == RUN) {
          s = CONTINUE;
          npom++;
        } else if (s == BREAK) {
          s = READY;
        }
        displayreset();        
      }
    }
  } else if (s == CONTINUE) {
    if (targetTime < millis()) {
      // Set next update for 1 second later
      targetTime = millis() + 1000;
  
      // Adjust the time values by adding 1 second
      ss++;              // Advance second
      if (ss == 60) {    // Check for roll-over
        ss = 0;          // Reset seconds to zero
        omm = mm;        // Save last minute time for display update
        mm++;            // Advance minute
      }
      displaytime(); 
    }    
  }
}
