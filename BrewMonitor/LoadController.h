#include <EEPROM.h>

template <class T> void eeGet(int ee,T& value) {
#ifdef ARDUINO_ARCH_STM32F1
  byte* p=(byte*)&value;
  unsigned int i;
  for(i=0;i<sizeof(value);i++)
    *p++=EEPROM.read(ee++);
#else
  EEPROM.get(ee, value);
#endif
}

template <class T> void eePut(int ee,T& value) {
#ifdef ARDUINO_ARCH_STM32F1
  const byte* p=(const byte*)&value;
  unsigned int i;
  for(i=0;i<sizeof(value);i++)
    EEPROM.update(ee++,*p++);
#else
  EEPROM.put(ee, value);
#endif
}

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
        
        eeGet(addr, controlMode);
        addr += sizeof(controlMode);
        eeGet(addr, targetTemp);
        addr += sizeof(targetTemp);
        eeGet(addr, allowedRange);
        addr += sizeof(allowedRange);
        eeGet(addr, powerControlCycleTime);
      } else {
        PRINT(F("LC No guard\n"));
      }
    }
    
    void save(void) {
      int addr = DATA_ADDR;
      
      eePut(addr, controlMode);
      addr += sizeof(controlMode);
      eePut(addr, targetTemp);
      addr += sizeof(targetTemp);
      eePut(addr, allowedRange);
      addr += sizeof(allowedRange);
      eePut(addr, powerControlCycleTime);
      eePut(GUARD_ADDR, GUARD_VALUE);
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
