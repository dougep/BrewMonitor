#include "Menus.h"

#define NUMITEMS(items) (sizeof(items)/sizeof(char*))

static const char *menuItems[] = { "Mode", "Target Temp", "Temp Band", "Duty Cycle" };
static const char *modeSubItems[] = { "Heating", "Cooling" };
static const char *targetSubItems[] = { "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25" };
static const char *bandSubItems[] = { "1", "2", "3", "4", "5" };
static const char *dutyCycleSubItems[] = { "On", "Off" };
static const char *dutyCycleOnSubItems[] = { "30", "60", "90", "120", "180", "300" };
static const char *dutyCycleOffSubItems[] = { "30", "60", "90", "120", "180", "300" };

class MenuHandler : public MenuCallback {
  private:
  MenuDisplay menuDisplay;
  LoadController *loadControl;
  Menu *menu;
  Menu *modeSub;
  Menu *targetSub;
  Menu *bandSub;
  Menu *dutyCycleSub;
  Menu *dutyCycleOnSub;
  Menu *dutyCycleOffSub;

  private:
  void handleModeSelection(const char *selected) {
    if (strcmp(selected, modeSubItems[0]) == 0) {
      loadControl->setControlMode(LoadController::Heating);
    } else {
      loadControl->setControlMode(LoadController::Cooling);
    }
  }

  void handleTargetSelection(const char *selected) {
    loadControl->setTargetTemp((unsigned)atoi(selected));
  }

  void handleBandSelection(const char *selected) {
  }

  void handleDutyCycleOnSelection(const char *selected) {
  }

  void handleDutyCycleOffSelection(const char *selected) {
  }

  void initSelectedMode(void) {
    modeSub->setSelectedIndex(loadControl->getControlMode() == LoadController::Heating ? 0 : 1);
  }

  void initSelectedTargetTemp(void) {
    unsigned target = loadControl->getTargetTemp();
    
    for (int i=0; i<NUMITEMS(targetSubItems); i++) {
      if (target <= atoi(targetSubItems[i])) {
        targetSub->setSelectedIndex(i);

        break;
      }
    }
  }

  public:
  MenuHandler(TFT_22_ILI9225 &tft, ButtonController &buttons, LoadController &lc)
  : menuDisplay(tft, buttons),
    loadControl(&lc) {
    menu = new Menu(menuItems, NUMITEMS(menuItems));
    modeSub = new Menu(modeSubItems, NUMITEMS(modeSubItems), this);
    targetSub = new Menu(targetSubItems, NUMITEMS(targetSubItems), this);
    bandSub = new Menu(bandSubItems, NUMITEMS(bandSubItems), this);
    dutyCycleSub = new Menu(dutyCycleSubItems, NUMITEMS(dutyCycleSubItems));
    dutyCycleOnSub = new Menu(dutyCycleOnSubItems, NUMITEMS(dutyCycleOnSubItems), this);
    dutyCycleOffSub = new Menu(dutyCycleOffSubItems, NUMITEMS(dutyCycleOffSubItems), this);
    menu->addSubMenu(0, modeSub);
    menu->addSubMenu(1, targetSub);
    menu->addSubMenu(2, bandSub);
    menu->addSubMenu(3, dutyCycleSub);
    dutyCycleSub->addSubMenu(0, dutyCycleOnSub);
    dutyCycleSub->addSubMenu(1, dutyCycleOffSub);
  }
  
  virtual ~MenuHandler()
  {
    delete menu;
    delete modeSub;
    delete targetSub;
    delete bandSub;
    delete dutyCycleSub;
    delete dutyCycleOnSub;
    delete dutyCycleOffSub;
  }
  
  void presentMenu(void) {
    initSelectedMode();
    initSelectedTargetTemp();

    menuDisplay.presentMenu(menu);
  }

  virtual void itemSelected(Menu *menu, const char *selected) {
    if (menu == modeSub) {
      handleModeSelection(selected);
    } else if (menu == targetSub) {
      handleTargetSelection(selected);
    } else if (menu == bandSub) {
      handleBandSelection(selected);
    } else if (menu == dutyCycleOnSub) {
      handleDutyCycleOnSelection(selected);
    } else if (menu == dutyCycleOffSub) {
      handleDutyCycleOffSelection(selected);
    }
  }
};
