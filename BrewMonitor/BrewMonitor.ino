// Include application, user and local libraries
#include "SPI.h"
#include <limits.h>

#define DEBUG

#ifdef DEBUG
  #define PRINT(...) Serial.print(__VA_ARGS__)
  #define PRINTLN(...) { Serial.print(__VA_ARGS__); Serial.print("\n"); }
  #define PRINTVAR(v) Serial.print("   "#v": " + String(v) + "\n")
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

#ifdef ARDUINO_ARCH_STM32F1
  #define LOAD_CONTROL_PIN PB8
  #define TEMP_SENSORS_PIN PB9
#else
  #define LOAD_CONTROL_PIN 10
  #define TEMP_SENSORS_PIN 9
#endif

#ifdef ARDUINO_ARCH_STM32F1
  #define TFT_LED PB1   // 0 if wired to +5V directly
  #define TFT_RS  PB5
  #define TFT_RST PB6
  #define TFT_CS  PB7

  #define BTN_UP PA0
  #define BTN_DOWN PA1
  #define BTN_SELECT PA2
  #define BTN_BACK PA3
#else
  #define TFT_LED 5   // 0 if wired to +5V directly
  #define TFT_CLK 13  // SCK
  #define TFT_SDI 11  // MOSI
  #define TFT_RS  4
  #define TFT_RST 3
  #define TFT_CS  2  // SS
#endif

#define SCREEN_TIMEOUT 60000UL  // Milliseconds
#define ORIENTATION 3
#define TFT_BRIGHTNESS 100 // Initial brightness of TFT backlight (optional)

TFT_22_ILI9225 tft(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
ButtonController buttons;
ChartDisplay chartDisplay(tft);
TempSensors sensors;
LoadController loadControl;

bool screenOnFlag = true;
unsigned long screenTimeoutStart = millis();

//============================================================
// Setup
void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  delay(1000);
#endif

  PRINT(F("Init Start\n"));

  tft.begin();
  tft.setOrientation(ORIENTATION);
  chartDisplay.init();
  sensors.init(TEMP_SENSORS_PIN);
  loadControl.init(LOAD_CONTROL_PIN);
  buttons.init(BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK);

  resetScreenTimeout();
  
  PRINT(F("Init Done\n"));
}

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

//============================================================
// Loop
void loop() {
  PRINT("Loop Start\n");

  do {
    float temps[3];

    sensors.getTemps(temps);
    PRINT("Temps:\n");
    PRINTVAR(temps[beer]);
    PRINTVAR(temps[coolant]);
    PRINTVAR(temps[air]);
    
    chartDisplay.addDataPoint(millis(), temps[beer], temps[coolant], temps[air]);

    loadControl.check(temps[beer]);

    if (screenTimedOut()) {
      screenOff();
    }

    for (int i=0; i<30; i++) {
      if (!screenIsOn()) {
        if (buttons.buttonPressed(ButtonAny)) {
          screenOn();
          
          resetScreenTimeout();
        }
      } else if (buttons.buttonPressed(ButtonSelect)) {
        handleMenu();

        chartDisplay.redraw();

        resetScreenTimeout();

        break;
      }

      delay(50);
    }
  }
  while(true);
}
