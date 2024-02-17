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

  typedef enum {
    Energised,
    Off
  } PowerControl;

  typedef enum {
    Active,
    Idle
  } State;

  private:
  class Settings {
    private:
    static const int GUARD_ADDR = 0;
    static const byte GUARD_VALUE = 0xAB;
    static const int DATA_ADDR = 1;
    
    public:
    ControlMode controlMode;
    unsigned targetTemp;
    unsigned allowedRange;
    unsigned powerControlDutyCycleOn;
    unsigned powerControlDutyCycleOff;

    Settings()
      : controlMode(Cooling),
        targetTemp(29),
        allowedRange(2),
        powerControlDutyCycleOn(30),
        powerControlDutyCycleOff(30) {
    }

    void load(void) {
      if (EEPROM.read(GUARD_ADDR) == GUARD_VALUE) {
        PRINTLN(F("LC Loading"));
        
        int addr = DATA_ADDR;
        
        eeGet(addr, controlMode);
        addr += sizeof(controlMode);
        eeGet(addr, targetTemp);
        addr += sizeof(targetTemp);
        eeGet(addr, allowedRange);
        addr += sizeof(allowedRange);
        eeGet(addr, powerControlDutyCycleOn);
        addr += sizeof(powerControlDutyCycleOn);
        eeGet(addr, powerControlDutyCycleOff);
      } else {
        PRINTLN(F("LC No guard"));
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
      eePut(addr, powerControlDutyCycleOn);
      addr += sizeof(powerControlDutyCycleOn);
      eePut(addr, powerControlDutyCycleOff);
      eePut(GUARD_ADDR, GUARD_VALUE);
    }
  };

  private:
  Settings settings;

  State state;
  PowerControl powerControl;
  int controlPinOn;
  int controlPinOff;
  unsigned long powerControlStartTime;
  
  public:
  LoadController()
    : state(Idle) {
  }

  void init(int onPin, int offPin) {
    initialisePowerControl(onPin, offPin);
    setIdle();
    initialiseSettings();
  }

  void check(float beerTemp) {
    PRINT(F("LC Check"));
    PRINTVAR(beerTemp);
    
    if (!updateState(beerTemp))
      updatePowerControl(beerTemp);
  }

  ControlMode getControlMode(void) {
    return settings.controlMode;
  }

  void setControlMode(ControlMode mode) {
    settings.controlMode = mode;
    settings.save();
  }

  unsigned getTargetTemp(void) {
    return settings.targetTemp;
  }

  void setTargetTemp(unsigned target) {
    settings.targetTemp = target;
    settings.save();
  }

  unsigned getTempRange(void) {
    return settings.allowedRange;
  }

  void setTempRange(unsigned range) {
    settings.allowedRange = range;
    settings.save();
  }

  unsigned getDutyCycleOn(void) {
    return settings.powerControlDutyCycleOn;
  }

  void setDutyCycleOn(unsigned cycle) {
    settings.powerControlDutyCycleOn = cycle;
    settings.save();
  }

  unsigned getDutyCycleOff(void) {
    return settings.powerControlDutyCycleOff;
  }

  void setDutyCycleOff(unsigned cycle) {
    settings.powerControlDutyCycleOff = cycle;
    settings.save();
  }

  PowerControl getPowerControlState(void) {
    return powerControl;
  }
  
  void setPowerControlOn(void) {
    digitalWrite(controlPinOn, HIGH);
    
    powerControl = Energised;
    powerControlStartTime = millis();
    
    PRINTLN(F("LC Power on"));
  }
  
  void setPowerControlOff(void) {
    digitalWrite(controlPinOn, LOW);
    
    powerControl = Off;
    powerControlStartTime = millis();
    
    PRINTLN(F("LC Power off"));
  }

  State getActiveState(void) {
    return state;
  }
  
  private:
  void initialisePowerControl(int onPin, int offPin) {
    controlPinOn = onPin;
    controlPinOff = offPin;

    digitalWrite(controlPinOn, LOW);
    digitalWrite(controlPinOff, LOW);
    pinMode(controlPinOn, OUTPUT);
    pinMode(controlPinOff, OUTPUT);
    
    setPowerControlOff();
  }

  void initialiseSettings(void) {
    settings.load();
  
    PRINTVAR(settings.controlMode);
    PRINTVAR(settings.targetTemp);
    PRINTVAR(settings.allowedRange);
    PRINTVAR(settings.powerControlDutyCycleOn);
    PRINTVAR(settings.powerControlDutyCycleOff);
  }

  void setIdle() {
    PRINTLN(F("LC Idle"));
    
    state = Idle;
  }

  void setActive() {
    PRINTLN(F("LC Active"));
    
    state = Active;
  }

  bool goalSatisfied(float beerTemp) {
    if (settings.controlMode == Heating) {
      return beerTemp >= (float)settings.targetTemp + (float)settings.allowedRange / 2.0;
    } else {
      return beerTemp <= (float)settings.targetTemp - (float)settings.allowedRange / 2.0;
    }
  }
  
  bool limitBreached(float beerTemp) {
    if (settings.controlMode == Heating) {
      return beerTemp < (float)settings.targetTemp - (float)settings.allowedRange / 2.0;
    } else {
      return beerTemp > (float)settings.targetTemp + (float)settings.allowedRange / 2.0;
    }
  }
  
  bool updateState(float beerTemp) {
    if (state == Active) {
      if (goalSatisfied(beerTemp)) {
        setIdle();
        setPowerControlOff();
        return true;
      }
    } else {
      if (limitBreached(beerTemp)) {
        setActive();
        setPowerControlOn();
        return true;
      }
    }

    return false;
  }

  void updatePowerControl(float beerTemp) {
    if (state == Active) {
      if (powerControl == Energised) {
        if ((millis() - powerControlStartTime >= settings.powerControlDutyCycleOn * 1000) && (settings.powerControlDutyCycleOff > 0)) {
          setPowerControlOff();
        }
      } else {
        if (millis() - powerControlStartTime >= settings.powerControlDutyCycleOff * 1000) {
          setPowerControlOn();
        }
      }
    }
  }
};
