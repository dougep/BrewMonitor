#include <TFT_22_ILI9225.h>

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

#define COLOR_RELAY_ON 0x9840

class ChartDisplay {
  private:
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
  
  private:
  static const unsigned long chartWidth = X_RANGE*60L*60L*1000L;

  private:
  TFT_22_ILI9225 &tft;
  
  byte *storage[3] = {};
  float minTemp[3] = { 99.0, 99.0, 99.0};
  float maxTemp[3] = {};
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
    
    tft.setFont(Terminal11x16);
    tft.drawText(temps[type].x, 18, String(temp, 1) + "  ", temps[type].colour);

    tft.setFont(Terminal6x8);
    tft.drawText(temps[type].x, 35, String(minTemp[type], 1) + "/" + String(maxTemp[type], 1) + "  ", temps[type].colour);
  }

  public:
  ChartDisplay(TFT_22_ILI9225 &tft)
   : tft(tft),
     startTime(millis()),
     barX(0),
     storage({0}) {
  }
  
  void init(void) {
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
