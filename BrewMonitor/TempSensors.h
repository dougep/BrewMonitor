#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_SENSORS_PIN 9

class TempSensors {
  private:
  static const byte SENSOR_ADDR_BEER[];
  static const byte SENSOR_ADDR_COOLANT[];
  static const byte SENSOR_ADDR_AIR[];

  private:
  OneWire *ds;
  DallasTemperature *rawSensors;
  
  public:
  void init(void) {
    ds = new OneWire(TEMP_SENSORS_PIN);
    rawSensors = new DallasTemperature(ds);

    rawSensors->begin();
  }

  void getTemps(float *temps) {
    rawSensors->requestTemperatures();
    temps[beer] = rawSensors->getTempC(SENSOR_ADDR_BEER);
    temps[coolant] = rawSensors->getTempC(SENSOR_ADDR_COOLANT);
    temps[air] = rawSensors->getTempC(SENSOR_ADDR_AIR);
  }
};

const byte TempSensors::SENSOR_ADDR_COOLANT[] = { 0x28, 0xf2, 0xe3, 0x95, 0x0a, 0x00, 0x00, 0x9c };
const byte TempSensors::SENSOR_ADDR_AIR[] = { 0x28, 0x8a, 0xf7, 0x95, 0x0a, 0x00, 0x00, 0xc6 };
const byte TempSensors::SENSOR_ADDR_BEER[] = { 0x28, 0xc6, 0xed, 0x95, 0x0a, 0x00, 0x00, 0x0f };