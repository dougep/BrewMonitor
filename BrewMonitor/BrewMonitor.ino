// Include application, user and local libraries
#include "SPI.h"
#include <limits.h>

#undef DEBUG

#ifdef DEBUG
  #define PRINT(...) Serial.print(__VA_ARGS__)
  #define PRINTVAR(v) Serial.print("   "#v": " + String(v) + "\n")
#else
  #define PRINT(...)
  #define PRINTVAR(v)
#endif

#include "TempType.h"
#include "LoadController.h"
#include "ChartDisplay.h"
#include "TempSensors.h"
#include "MenuHandler.h"

//============================================================
// Variables and constants

#define RELAY_PIN 8
#define LOAD_CONTROL_PIN 10
#define TEMP_SENSORS_PIN 9

#ifdef ARDUINO_ARCH_STM32F1
  blah
#else
  #define TFT_RST 3
  #define TFT_RS  4
  #define TFT_CS  2  // SS
  #define TFT_SDI 11  // MOSI
  #define TFT_CLK 13  // SCK
  #define TFT_LED 5   // 0 if wired to +5V directly
#endif

#define ORIENTATION 3
#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

TFT_22_ILI9225 tft(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
ChartDisplay chartDisplay(tft);
TempSensors sensors;
LoadController loadControl;

//============================================================
// Setup
void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

  PRINT(F("Init Start\n"));

  tft.begin();
  tft.setOrientation(ORIENTATION);
  chartDisplay.init();
  sensors.init(TEMP_SENSORS_PIN);
  loadControl.init(LOAD_CONTROL_PIN);
  pinMode(RELAY_PIN, INPUT_PULLUP);

  PRINT(F("Init Done\n"));
}

void handleMenu() {
  MenuHandler handler(tft);

  handler.presentMenu();
}

//============================================================
// Loop
void loop() {
  PRINT("Loop Start\n");

  do {
    float temps[3];

    sensors.getTemps(temps);
    
    chartDisplay.addDataPoint(millis(), digitalRead(RELAY_PIN) == LOW, temps[beer], temps[coolant], temps[air]);

    loadControl.check(temps[beer]);
    
    delay(3000);

    handleMenu();

    delay(3000);
  }
  while(true);
}
