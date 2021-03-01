// Include application, user and local libraries
#include "SPI.h"
#include <limits.h>

#define DEBUG

#ifdef DEBUG
  #define PRINT(...) Serial.print(__VA_ARGS__)
  #define PRINTVAR(v) Serial.print("   "#v": " + String(v) + "\n")
#else
  #define PRINT(...)
#endif

#include "TempType.h"
#include "LoadController.h"
#include "Display.h"
#include "TempSensors.h"

//============================================================
// Variables and constants

#define RELAY_PIN 8

Display lcd;
TempSensors sensors;
LoadController loadControl;

//============================================================
// Setup
void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

  PRINT("Init Start\n");
  
  lcd.init();
  sensors.init();
  pinMode(RELAY_PIN, INPUT_PULLUP);

  PRINT("Init Done\n");
}

//============================================================
// Loop
void loop() {
  PRINT("Loop Start\n");

  do {
    float temps[3];

    sensors.getTemps(temps);
    
    lcd.addDataPoint(millis(), digitalRead(RELAY_PIN) == LOW, temps[beer], temps[coolant], temps[air]);

    loadControl.check(temps[beer]);
    
    delay(5000);
  }
  while(true);
}
