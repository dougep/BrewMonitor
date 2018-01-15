// Include application, user and local libraries
#include "SPI.h"
#include <TFT_22_ILI9225.h>

#define DEBUG

#define ORIENTATION 3
#define TFT_RST 3
#define TFT_RS  4
#define TFT_CS  2  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 5   // 0 if wired to +5V directly

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

// Variables and constants
uint16_t width, height;

void initDisplay(void) {
  tft.begin();
  tft.setOrientation(ORIENTATION);
  width = tft.maxX();
  height = tft.maxY();

  tft.drawRectangle(0, 0, width - 1, height - 1, COLOR_WHITE);
  tft.setFont(Terminal12x16);
  tft.drawText(10,10,"Beer:     21.8", COLOR_BLUE);
  tft.drawText(10,30,"Coolant: 5.2", COLOR_GREEN);
  tft.drawText(10,50,"Air:       37.5", COLOR_RED);
}

void initSensors(void) {
  
}

// Setup
void setup() {
  initDisplay();
  initSensors();
#ifdef DEBUG
  Serial.begin(9600);
#endif
}

// Loop
void loop() {
  do {
    Serial.print("Loop Start\n");
    delay(10000);
  }
  while(true);
}
