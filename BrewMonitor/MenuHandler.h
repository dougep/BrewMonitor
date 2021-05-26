#include "Menus.h"

#define NUMITEMS(items) (sizeof(items)/sizeof(char*))

static const char *menuItems[] = { "Mode", "Target Temp", "Temp Range", "Duty Cycle", "Power Control" };
static const char *modeSubItems[] = { "Heating", "Cooling" };
static const char *targetTempSubItems[] = { "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25" };
static const char *tempRangeSubItems[] = { "1", "2", "3", "4", "5" };
static const char *dutyCycleSubItems[] = { "On", "Off" };
static const char *dutyCycleOnSubItems[] = { "30", "60", "90", "120", "180", "300" };
static const char *dutyCycleOffSubItems[] = { "30", "60", "90", "120", "180", "300" };
static const char *powerControlSubItems[] = { "On", "Off" };

class MenuHandler : public MenuCallback {
  private:
  MenuDisplay menuDisplay;
  LoadController *loadControl;
  Menu *menu;
  Menu *modeSub;
  Menu *targetTempSub;
  Menu *tempRangeSub;
  Menu *dutyCycleSub;
  Menu *dutyCycleOnSub;
  Menu *dutyCycleOffSub;
  Menu *powerControlSub;

  private:
  void handleModeSelection(const char *selected) {
    if (strcmp(selected, modeSubItems[0]) == 0) {
      loadControl->setControlMode(LoadController::Heating);
    } else {
      loadControl->setControlMode(LoadController::Cooling);
    }
  }

  void handleTargetTempSelection(const char *selected) {
    loadControl->setTargetTemp((unsigned)atoi(selected));
  }

  void handleTempRangeSelection(const char *selected) {
    loadControl->setTempRange((unsigned)atoi(selected));
  }

  void handleDutyCycleOnSelection(const char *selected) {
    loadControl->setDutyCycleOn((unsigned)atoi(selected));
  }

  void handleDutyCycleOffSelection(const char *selected) {
    loadControl->setDutyCycleOff((unsigned)atoi(selected));
  }

  void handlePowerControlSelection(const char *selected) {
    if (strcmp(selected, powerControlSubItems[0]) == 0) {
      loadControl->setPowerControlOn();
    } else {
      loadControl->setPowerControlOff();
    }
  }

  int findEntry(int val, const char **list, unsigned count) {
    for (int i=0; i<count; i++) {
      if (val <= atoi(list[i])) {
        return i;
      }
    }

    return -1;
  }

  void initSelectedMode(void) {
    modeSub->setSelectedIndex(loadControl->getControlMode() == LoadController::Heating ? 0 : 1);
  }

  void initSelectedTargetTemp(void) {
    unsigned targetTemp = loadControl->getTargetTemp();
    int index;

    if ((index = findEntry(targetTemp, targetTempSubItems, NUMITEMS(targetTempSubItems))) >= 0) {
        targetTempSub->setSelectedIndex(index);
    }
  }

  void initTempRange(void) {
    unsigned tempRange = loadControl->getTempRange();
    int index;

    if ((index = findEntry(tempRange, tempRangeSubItems, NUMITEMS(tempRangeSubItems))) >= 0) {
        tempRangeSub->setSelectedIndex(index);
    }
  }

  void initDutyCycleOn(void) {
    unsigned cycle = loadControl->getDutyCycleOn();
    int index;

    if ((index = findEntry(cycle, dutyCycleOnSubItems, NUMITEMS(dutyCycleOnSubItems))) >= 0) {
        dutyCycleOnSub->setSelectedIndex(index);
    }
  }

  void initDutyCycleOff(void) {
    unsigned cycle = loadControl->getDutyCycleOff();
    int index;

    if ((index = findEntry(cycle, dutyCycleOffSubItems, NUMITEMS(dutyCycleOffSubItems))) >= 0) {
        dutyCycleOffSub->setSelectedIndex(index);
    }
  }

  void initPowerControl(void) {
    powerControlSub->setSelectedIndex(loadControl->getPowerControlState() == LoadController::Energised ? 0 : 1);
  }

  public:
  MenuHandler(TFT_22_ILI9225 &tft, ButtonController &buttons, LoadController &lc)
  : menuDisplay(tft, buttons),
    loadControl(&lc) {
    menu = new Menu(menuItems, NUMITEMS(menuItems));
    modeSub = new Menu(modeSubItems, NUMITEMS(modeSubItems), this);
    targetTempSub = new Menu(targetTempSubItems, NUMITEMS(targetTempSubItems), this);
    tempRangeSub = new Menu(tempRangeSubItems, NUMITEMS(tempRangeSubItems), this);
    dutyCycleSub = new Menu(dutyCycleSubItems, NUMITEMS(dutyCycleSubItems));
    dutyCycleOnSub = new Menu(dutyCycleOnSubItems, NUMITEMS(dutyCycleOnSubItems), this);
    dutyCycleOffSub = new Menu(dutyCycleOffSubItems, NUMITEMS(dutyCycleOffSubItems), this);
    powerControlSub = new Menu(powerControlSubItems, NUMITEMS(powerControlSubItems), this);
    menu->addSubMenu(0, modeSub);
    menu->addSubMenu(1, targetTempSub);
    menu->addSubMenu(2, tempRangeSub);
    menu->addSubMenu(3, dutyCycleSub);
    menu->addSubMenu(4, powerControlSub);
    dutyCycleSub->addSubMenu(0, dutyCycleOnSub);
    dutyCycleSub->addSubMenu(1, dutyCycleOffSub);
  }
  
  virtual ~MenuHandler()
  {
    delete menu;
    delete modeSub;
    delete targetTempSub;
    delete tempRangeSub;
    delete dutyCycleSub;
    delete dutyCycleOnSub;
    delete dutyCycleOffSub;
    delete powerControlSub;
  }
  
  void presentMenu(void) {
    initSelectedMode();
    initSelectedTargetTemp();
    initTempRange();
    initDutyCycleOn();
    initDutyCycleOff();
    initPowerControl();

    menuDisplay.presentMenu(menu);
  }

  virtual void itemSelected(Menu *menu, const char *selected) {
    if (menu == modeSub) {
      handleModeSelection(selected);
    } else if (menu == targetTempSub) {
      handleTargetTempSelection(selected);
    } else if (menu == tempRangeSub) {
      handleTempRangeSelection(selected);
    } else if (menu == dutyCycleOnSub) {
      handleDutyCycleOnSelection(selected);
    } else if (menu == dutyCycleOffSub) {
      handleDutyCycleOffSelection(selected);
    } else if (menu == powerControlSub) {
      handlePowerControlSelection(selected);
    }
  }
};
