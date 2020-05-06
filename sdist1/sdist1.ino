#include "Adafruit_VL53L0X.h"
#include <MD_Parola.h>
//   https://github.com/MajicDesigns/MD_Parola
#include <MD_MAX72xx.h>
//   https://github.com/MajicDesigns/MD_MAX72xx
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 4  // Number of modules connected
#define CLK_PIN   16   // SPI SCK pin on UNO
#define DATA_PIN  12   // SPI MOSI pin on UNO
#define CS_PIN    14   // connected to pin 10 on UNO
#define TRIG_DIST 1800
#define MIN_DIST 300
#define DISP_SPEED 30
#define MAXMESS 5
#define BANNERINT 5 //minutes
const bool DBGOUT=false;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();


//MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);  // SPI config
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// sets scrolling direction if slider in middle at start
textEffect_t scrollEffect = PA_SCROLL_RIGHT;

textPosition_t scrollAlign = PA_LEFT;  // how to aligh the text
int needsRst = 1;
int dClear = 0;
bool Startup = true;
int scrollPause = 50; // ms of pause after finished displaying message

#define  BUF_SIZE  75  // Maximum of 75 characters
char StartMsg [BUF_SIZE] = {"PDA V1.0"};
char curMessage[MAXMESS][BUF_SIZE] = {{ "DON'T STAND SO CLOSE TO ME!!"}, {"Social distancing is 6 feet."}, {"Please get off my a$$."}, {"Please stand back 6'" }, {"I don't want the Rona! Please back off."}}; // used to hold current message
char startupMessage[BUF_SIZE] = {"WWW.PERSONALDISTANCEALERT.COM"};
char bannerMessage[BUF_SIZE] = {"WWW.PERSONALDISTANCEALERT.COM"};
int myIndx = 0;
unsigned long StartTime;
unsigned long EndTime;



void setup()
{

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  Serial.println("Personal Distance Alert V 1.0");
  // power
  P.begin();  // Start Parola
  // configure Parola
  P.setZoneEffect(0, true, PA_FLIP_LR);
  if (!lox.begin()) {
    if (DBGOUT) {
    Serial.println(F("Failed to boot VL53L0X"));
    }
    while (1);
  }
}

int SetNewMessage(int ndx) {
  int retval = ndx + 1;
  if (retval > MAXMESS) {
    retval = 0;
  }
  //Serial.printf("Setting new message %i\n", retval);
  P.displayText(&curMessage[retval][0], PA_RIGHT, DISP_SPEED, scrollPause, PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
  //P.displayScroll(&curMessage[retval][0],PA_RIGHT, PA_SCROLL_RIGHT, DISP_SPEED);
  return retval;
}

void ResetBannerTimer() {
  StartTime = millis();
  EndTime = StartTime + (BANNERINT * 60 * 1000);
  if (DBGOUT) {
  Serial.printf("StartTime is %i EndTime is %i\n", StartTime, EndTime);
  }
}

bool CheckBannerTimer() {
  if (DBGOUT) {
  Serial.printf("EndTime is %i CurTime is %i\n", EndTime, millis());
  }
  return millis() > EndTime;
}

void TriggerDisplay() {
  bool Done = false;
  myIndx = SetNewMessage(myIndx);
  Animator();
  needsRst = 1;
}

void Animator() {
  bool Done = false;
  do {
    if (P.displayAnimate()) {
      Done = true;
    }
    delay(10);
  } while (!Done);
}

void DisplayBanner() {
  bool Done = false;
  P.displayText(bannerMessage, PA_RIGHT, DISP_SPEED, scrollPause, PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
  Animator();
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  if (Startup) {
    bool Done = false;
    P.displayText(startupMessage, PA_RIGHT, DISP_SPEED, scrollPause, PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    Animator();
    Startup = false;
  }
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    int rm = measure.RangeMilliMeter;

    if ((rm > MIN_DIST) && (rm < TRIG_DIST)) {
      if (DBGOUT) {
      Serial.printf("Trigger range is %i mm\n", rm);
      }
      TriggerDisplay();
      ResetBannerTimer();

    } else {
      if (rm > 0) {
        //Serial.printf("Range is %i mm\n", rm);
      }
    }
  }
  delay(500); //Wait for half a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for half a second
  if (CheckBannerTimer()) {
    DisplayBanner();
    ResetBannerTimer();
  }

}
