#include <EEPROM.h>

class LoadController {
  public:
  typedef enum {
    Heating,
    Cooling
  } ControlMode;

  private:
  typedef enum {
    Active,
    Idle
  } State;

  typedef enum {
    Energised,
    Off
  } PowerControl;

  class Settings {
    private:
    static const int GUARD_ADDR = 0;
    static const byte GUARD_VALUE = 0xAA;
    static const int DATA_ADDR = 1;
    
    public:
    ControlMode controlMode;
    float targetTemp;
    float allowedRange;
    unsigned long powerControlCycleTime;

    Settings()
      : controlMode(Cooling),
        targetTemp(29),
        allowedRange(2),
        powerControlCycleTime(30000) {
    }

    void load(void) {
      if (EEPROM.read(GUARD_ADDR) == GUARD_VALUE) {
        PRINT(F("LC Loading\n"));
        
        int addr = DATA_ADDR;
        
        EEPROM.get(addr, controlMode);
        addr += sizeof(controlMode);
        EEPROM.get(addr, targetTemp);
        addr += sizeof(targetTemp);
        EEPROM.get(addr, allowedRange);
        addr += sizeof(allowedRange);
        EEPROM.get(addr, powerControlCycleTime);
      } else {
        PRINT(F("LC No guard\n"));
      }
    }
    
    void save(void) {
      int addr = DATA_ADDR;
      
      EEPROM.put(addr, controlMode);
      addr += sizeof(controlMode);
      EEPROM.put(addr, targetTemp);
      addr += sizeof(targetTemp);
      EEPROM.put(addr, allowedRange);
      addr += sizeof(allowedRange);
      EEPROM.put(addr, powerControlCycleTime);
      EEPROM.put(GUARD_ADDR, GUARD_VALUE);
    }
  };

  private:
  Settings settings;

  State state;
  PowerControl powerControl;
  int controlPin;
  unsigned long powerControlStartTime;
  
  public:
  LoadController()
    : state(Idle) {
  }

  void init(int pin) {
    initialisePowerControl(pin);
    setIdle();
    initialiseSettings();
  }

  void check(float beerTemp) {
    PRINT(F("LC Check"));
    PRINTVAR(beerTemp);
    
    if (!updateState(beerTemp))
      updatePowerControl(beerTemp);
  }

  private:
  void initialisePowerControl(int pin) {
    controlPin = pin;
    pinMode(controlPin, OUTPUT);
    setPowerControlOff();
  }

  void initialiseSettings(void) {
    settings.load();
  
    PRINTVAR(settings.controlMode);
    PRINTVAR(settings.targetTemp);
    PRINTVAR(settings.allowedRange);
    PRINTVAR(settings.powerControlCycleTime);
}

  void setPowerControlOn(void) {
    digitalWrite(controlPin, HIGH);
    powerControl = Energised;
    powerControlStartTime = millis();
    
    PRINT(F("LC Power on\n"));
  }
  
  void setPowerControlOff(void) {
    digitalWrite(controlPin, LOW);
    powerControl = Off;
    powerControlStartTime = millis();
    
    PRINT(F("LC Power off\n"));
  }

  void setIdle() {
    PRINT(F("LC Idle\n"));
    
    state = Idle;
  }

  void setActive() {
    PRINT(F("LC Active\n"));
    
    state = Active;
  }

  bool goalSatisfied(float beerTemp) {
    if (settings.controlMode == Heating) {
      return beerTemp >= settings.targetTemp + settings.allowedRange / 2;
    } else {
      return beerTemp <= settings.targetTemp - settings.allowedRange / 2;
    }
  }
  
  bool limitBreached(float beerTemp) {
    if (settings.controlMode == Heating) {
      return beerTemp < settings.targetTemp - settings.allowedRange / 2;
    } else {
      return beerTemp > settings.targetTemp + settings.allowedRange / 2;
    }
  }
  
  bool updateState(float beerTemp) {
    if (state == Active) {
      if (goalSatisfied(beerTemp)) {
        setIdle();
        setPowerControlOff();
      }
    } else {
      if (limitBreached(beerTemp)) {
        setActive();
        setPowerControlOn();
      }
    }
  }

  void updatePowerControl(float beerTemp) {
    if (state == Active) {
      if (millis() - powerControlStartTime >= settings.powerControlCycleTime) {
        if (powerControl == Energised) {
          setPowerControlOff();
        } else {
          setPowerControlOn();
        }
      }
    }
  }
};
