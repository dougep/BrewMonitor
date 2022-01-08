// Include application, user and local libraries
#include "SPI.h"
#include <limits.h>

#undef DEBUG

#ifdef DEBUG
  #define PRINT(...) Serial.print(__VA_ARGS__)
  #define PRINTLN(...) { Serial.print(__VA_ARGS__); Serial.print("\n"); }
  #define PRINTVAR(v) { Serial.print(__PRETTY_FUNCTION__); Serial.print(":   "#v": " + String(v) + "\n"); }
#else
  #define PRINT(...)
  #define PRINTLN(...)
  #define PRINTVAR(v)
#endif

#include "TempType.h"
#include "LoadController.h"
#include "ChartDisplay.h"
#include "TempSensors.h"
#include "Buttons.h"
#include "MenuHandler.h"

//============================================================
// Variables and constants

#define LOAD_CONTROL_PIN_ON PB3
#define LOAD_CONTROL_PIN_OFF PB4

#define TEMP_SENSORS_PIN PB9

#define TFT_LED PB1   // 0 if wired to +5V directly
#define TFT_RS  PB5
#define TFT_RST PB6
#define TFT_CS  PB7

#define BTN_UP PA0
#define BTN_DOWN PA1
#define BTN_SELECT PA2
#define BTN_BACK PA3

#define SCREEN_TIMEOUT 60000UL  // Milliseconds
#define TEMPS_TIMEOUT 4000UL  // Milliseconds
#define ORIENTATION 3
#define TFT_BRIGHTNESS 100 // Initial brightness of TFT backlight (optional)

//============================================================
// Globals
TFT_22_ILI9225 tft(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
ButtonController buttons;
ChartDisplay chartDisplay(tft);
TempSensors sensors;
LoadController loadControl;

bool screenOnFlag = true;
bool waitingForTemps = false;
unsigned long screenTimeoutStart = millis();
unsigned long tempsTimeoutStart = millis();

void handleMenu() {
  MenuHandler handler(tft, buttons, loadControl);

  handler.presentMenu();
}

void screenOn(void) {
  tft.setBacklight(true);
  tft.setDisplay(true);
  screenOnFlag = true;
}

void screenOff(void) {
  tft.setBacklight(false);
  tft.setDisplay(false);
  screenOnFlag = false;
}

bool screenIsOn(void) {
  return screenOnFlag;
}

void resetScreenTimeout(void) {
  screenTimeoutStart = millis();
}

bool screenTimedOut(void) {
  return millis() - screenTimeoutStart >= SCREEN_TIMEOUT;
}

void resetTempsTimeout(void) {
  tempsTimeoutStart = millis();
}

bool tempsReady(void) {
  return waitingForTemps && sensors.tempsReady();
}

bool tempsTimedOut(void) {
  return millis() - tempsTimeoutStart >= TEMPS_TIMEOUT;
}

void requestTemps(void) {
  PRINTLN("Request temps");
  
  sensors.requestTemps();

  waitingForTemps = true;

  resetTempsTimeout();
}

void updateTemps(void) {
  float temps[3];

  PRINTLN("Update temps");
  
  sensors.getTemps(temps);

  chartDisplay.addDataPoint(millis(), temps[beer], temps[coolant], temps[air], 
      loadControl.getActiveState() == LoadController::Active);
      
  loadControl.check(temps[beer]);

  waitingForTemps = false;
}

//============================================================
// Setup
void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  delay(1000);
#endif

  PRINTLN(F("Init Start"));

  tft.begin();
  tft.setOrientation(ORIENTATION);
  chartDisplay.init();
  sensors.init(TEMP_SENSORS_PIN);
  loadControl.init(LOAD_CONTROL_PIN_ON, LOAD_CONTROL_PIN_OFF);
  buttons.init(BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK);

  resetScreenTimeout();
  resetTempsTimeout();
  waitingForTemps = false;

  PRINTLN(F("Init Done"));
}

//============================================================
// Loop
void loop() {
  if (tempsReady()) {
    updateTemps();
  } else if (tempsTimedOut()) {
    requestTemps();
  }

  if (screenIsOn() && screenTimedOut()) {
    screenOff();
  } else if (!screenIsOn()) {
    if (buttons.buttonPressed(ButtonAny)) {
      screenOn();
      
      resetScreenTimeout();
    }
  } else if (buttons.buttonPressed(ButtonAny)) {
    handleMenu();

    chartDisplay.redraw();

    resetScreenTimeout();
  }
}
