// Include application, user and local libraries
#include "SPI.h"
#include <limits.h>
#include <TFT_22_ILI9225.h>

#define DEBUG

#ifdef DEBUG
  #define PRINT(...) Serial.print(__VA_ARGS__)
  #define PRINTVAR(v) Serial.print("   "#v": " + String(v) + "\n")
#else
  #define PRINT(...)
#endif

#define ORIENTATION 3
#define TFT_RST 3
#define TFT_RS  4
#define TFT_CS  2  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 5   // 0 if wired to +5V directly
#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)
#define COLOR_RELAY_ON 0x9840

#define RELAY_PIN 8

#define X_RANGE 12
#define Y_RANGE 4

#define X_ZERO 5
#define Y_ZERO (height-5-1)
#define X_HOUR ((width-X_ZERO-1)/X_RANGE)
#define X_HALF (X_HOUR/2)
#define X_QUARTER (X_HOUR/4)
#define X_PIXELS (X_RANGE*X_HOUR)
#define Y_10DEG ((Y_ZERO+1-44)/Y_RANGE)
#define Y_5DEG (Y_10DEG/2)
#define Y_TOP (Y_ZERO-Y_RANGE*Y_10DEG)
#define Y_BOTTOM (height-1)

typedef enum {
  beer = 0,
  coolant = 1,
  air = 2
} TempType;

class TextDetails {
  public:
  TextDetails(String text, unsigned x, unsigned colour)
  : text(text),
    x(x),
    colour(colour)
  { }

  String    text;
  unsigned  x;
  unsigned  colour;
};

class Display {
  private:
  static const unsigned long chartWidth = X_RANGE*60L*60L*1000L;

  private:
  byte *storage[3] = {};
  float minTemp[3] = { 99.0, 99.0, 99.0};
  float maxTemp[3] = {};
  TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
  TextDetails temps[3] = {
    TextDetails("Beer", 0, COLOR_BLUE),
    TextDetails("Coolant", 68, COLOR_GREEN),
    TextDetails("Air", 160, COLOR_RED)
  };

  unsigned width, height;
  unsigned long startTime;
  unsigned barX;

  private:
  void drawAxes(void) {
    tft.drawLine(X_ZERO, Y_TOP, X_ZERO, Y_BOTTOM, COLOR_YELLOW);
    tft.drawLine(0, Y_ZERO, X_ZERO+24*X_HOUR, Y_ZERO, COLOR_YELLOW);

    for (int x=X_ZERO+X_HOUR; x<=X_ZERO+24*X_HOUR; x+=X_HOUR) {
      tft.drawLine(x, Y_ZERO, x, Y_ZERO+5, COLOR_YELLOW);
      tft.drawLine(x-X_HALF, Y_ZERO, x-X_HALF, Y_ZERO+3, COLOR_YELLOW);
    }

    for (int y=Y_ZERO-Y_10DEG; y>=Y_ZERO-4*Y_10DEG; y-=Y_10DEG) {
      tft.drawLine(0, y, X_ZERO, y, COLOR_YELLOW);
      tft.drawLine(2, y+Y_5DEG, X_ZERO, y+Y_5DEG, COLOR_YELLOW);
    }
  }

  void advanceBar(unsigned long timestamp, bool relayOn) {
    unsigned newX = (float)((timestamp-startTime) % chartWidth) / chartWidth * X_PIXELS + X_ZERO;

    if (newX != barX) {
      if (barX) {
        tft.drawLine(barX+1, Y_TOP, barX+1, Y_ZERO-1, relayOn ? COLOR_RELAY_ON : COLOR_BLACK);
      }

      barX = newX;
      tft.drawLine(barX+1, Y_TOP, barX+1, Y_ZERO-1, COLOR_AZUR);
    }
  }

  unsigned tempToY(float temp) {
    return Y_ZERO - (unsigned)(temp / (Y_RANGE*10) * (Y_RANGE*Y_10DEG));
  }
  
  void plotPoints(float beerTemp, float coolantTemp, float airTemp) {
    if (barX > X_ZERO) {
      tft.drawPixel(barX, tempToY(beerTemp), temps[beer].colour);
      tft.drawPixel(barX, tempToY(coolantTemp), temps[coolant].colour);
      tft.drawPixel(barX, tempToY(airTemp), temps[air].colour);
    }
  }

  byte tempToStoreVal(float temp) {
    return (byte)(temp * 5.0);
  }

  float tempFromStoreVal(byte val) {
    return (float)val / 5.0;
  }

  void initMinMax(void) {
    storage[beer] = new byte[X_PIXELS];
    storage[coolant] = new byte[X_PIXELS];
    storage[air] = new byte[X_PIXELS];
    for (int i=0; i<X_PIXELS; i++) {
      storage[beer][i] = storage[coolant][i] = storage[air][i] = 255;
    }
  }

  void updateMinMax(TempType type, float temp) {
    storage[type][barX - X_ZERO] = min(254, tempToStoreVal(temp));
    byte minT = 255;
    byte maxT = 0;
    
    for (int i=0; i<X_PIXELS; i++) {
      byte val = storage[type][i];
      
      if (val == 255)
        break;

      if (minT > val)
        minT = val;
      if (maxT < val)
        maxT = val;
    }

    minTemp[type] = tempFromStoreVal(minT);
    maxTemp[type] = tempFromStoreVal(maxT);
  }

  void updateTemp(TempType type, float temp) {
    updateMinMax(type, temp);
    
    tft.setFont(Terminal12x16);
    tft.drawText(temps[type].x, 18, String(temp, 1) + "  ", temps[type].colour);

    tft.setFont(Terminal6x8);
    tft.drawText(temps[type].x, 35, String(minTemp[type], 1) + "/" + String(maxTemp[type], 1) + "  ", temps[type].colour);
  }

  public:
  Display()
   : startTime(millis()),
     barX(0),
     storage({0}) {
  }
  
  void init(void) {
    tft.begin();
    tft.setOrientation(ORIENTATION);
    width = tft.maxX();
    height = tft.maxY();
  
    tft.setFont(Terminal12x16);

    tft.drawText(temps[beer].x, 0, temps[beer].text, temps[beer].colour);
    tft.drawText(temps[coolant].x, 0, temps[coolant].text, temps[coolant].colour);
    tft.drawText(temps[air].x, 0, temps[air].text, temps[air].colour);

    initMinMax();
    
    drawAxes();

    startTime = millis();
  }

  void addDataPoint(unsigned long timestamp, bool relayOn, float beerTemp, float coolantTemp, float airTemp) {
    updateTemp(beer, beerTemp);
    updateTemp(coolant, coolantTemp);
    updateTemp(air, airTemp);

    advanceBar(timestamp, relayOn);
    plotPoints(beerTemp, coolantTemp, airTemp);
  }
};

class TempSensors {
  private:
  typedef struct CurvePoiont {
    unsigned long resistance;
    float temp;
  } CurvePoint;

  static const CurvePoint curve[];
  static const unsigned tempInputs[];

  private:
  unsigned long getAnalogueIn(TempType type) {
    static const unsigned long samples = 20;
    unsigned long total = 0;
    
    for (int i=0; i<samples; i++) {
       total += analogRead(tempInputs[type]);
       delay(2);
    }

    return total / samples;
  }
  public:
  void init(void) {
    
  }

  float getTemp(TempType type) {
    unsigned long in = getAnalogueIn(type);
    unsigned long res = in * 10000UL / (1024UL - in);
    int i=0;

    while (curve[i].resistance < res) {
      i++;
    }

    float fraction = (float)(res - curve[i-1].resistance) / (float)(curve[i].resistance - curve[i-1].resistance);
        
    float temp = curve[i-1].temp + fraction * (curve[i].temp - curve[i-1].temp);

    return temp;
  }
};

// Variables and constants
static const TempSensors::CurvePoint TempSensors::curve[] = { 
  { 0, 51 },
  { 3525, 50.56 },
  { 3679, 49.44 },
  { 3838, 48.33 },
  { 4006, 47.22 },
  { 4182, 46.11 },
  { 4367, 45 },
  { 4561, 43.89 },
  { 4766, 42.78 },
  { 4981, 41.67 },
  { 5207, 40.56 },
  { 5447, 39.44 },
  { 5697, 38.33 },
  { 5960, 37.22 },
  { 6238, 36.11 },
  { 6530, 35 },
  { 6838, 33.89 },
  { 7163, 32.78 },
  { 7505, 31.67 },
  { 7866, 30.56 },
  { 8251, 29.44 },
  { 8653, 28.33 },
  { 9078, 27.22 },
  { 9526, 26.11 },
  { 10000, 25 },
  { 10501, 23.89 },
  { 11030, 22.78 },
  { 11590, 21.67 },
  { 12182, 20.56 },
  { 12814, 19.44 },
  { 13478, 18.33 },
  { 14180, 17.22 },
  { 14925, 16.11 },
  { 15714, 15 },
  { 16550, 13.89 },
  { 17437, 12.78 },
  { 18378, 11.67 },
  { 19376, 10.56 },
  { 20446, 9.44 },
  { 21573, 8.33 },
  { 22770, 7.22 },
  { 24042, 6.11 },
  { 25395, 5 },
  { 26834, 3.89 },
  { 28365, 2.78 },
  { 29996, 1.67 },
  { 31732, 0.56 },
  { 33599, -0.56 },
  { UINT_MAX, -1 }
};

static const unsigned TempSensors::tempInputs[] = { A0, A1, A2 };

Display lcd;
TempSensors sensors;

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

// Loop
void loop() {
  PRINT("Loop Start\n");

  do {
    lcd.addDataPoint(millis(), digitalRead(RELAY_PIN) == LOW, sensors.getTemp(beer), sensors.getTemp(coolant), sensors.getTemp(air));
    
    delay(5000);
  }
  while(true);
}
