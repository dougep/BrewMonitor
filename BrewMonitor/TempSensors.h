#include <DallasTemperature.h>

class TempSensors {
  private:
  static const byte SENSOR_ADDR_BEER[];
  static const byte SENSOR_ADDR_COOLANT[];
  static const byte SENSOR_ADDR_AIR[];

  private:
  OneWire *ds;
  DallasTemperature *rawSensors;
  
  public:
  TempSensors() {
  }

  void init(int pin) {
    ds = new OneWire(pin);
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

const byte TempSensors::SENSOR_ADDR_AIR[] = { 0x28, 0xee, 0xa0, 0xce, 0x15, 0x21, 0x01, 0x1f };
const byte TempSensors::SENSOR_ADDR_COOLANT[] = { 0x28, 0x91, 0x40, 0xc9, 0x15, 0x21, 0x01, 0xff };
const byte TempSensors::SENSOR_ADDR_BEER[] = { 0x28, 0xd5, 0xdb, 0xc3, 0x15, 0x21, 0x01, 0x8a };
