#include "Menus.h"

#define NUMITEMS(items) (sizeof(items)/sizeof(char*))

static const char *menuItems[] = { "Mode", "Target", "Band", "Duty Cycle" };
static const char *modeSubItems[] = { "Heating", "Cooling" };
static const char *targetSubItems[] = { "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25" };
static const char *bandSubItems[] = { "1", "2", "3", "4", "5" };
static const char *dutyCycleSubItems[] = { "On", "Off" };
static const char *dutyCycleOnSubItems[] = { "30", "60", "90", "120", "180", "300" };
static const char *dutyCycleOffSubItems[] = { "30", "60", "90", "120", "180", "300" };

class MenuHandler : public MenuCallback {
  private:
  virtual void itemSelected(Menu *menu, const char *selected) {
      
  }

  MenuDisplay menuDisplay;

  public:
  MenuHandler(TFT_22_ILI9225 &tft, ButtonController &buttons)
  : menuDisplay(tft, buttons) {
    
  }
  
  void presentMenu(void) {
    Menu menu(menuItems, NUMITEMS(menuItems));
    Menu modeSub(modeSubItems, NUMITEMS(modeSubItems), this);
    Menu targetSub(targetSubItems, NUMITEMS(targetSubItems), this);
    Menu bandSub(bandSubItems, NUMITEMS(bandSubItems), this);
    Menu dutyCycleSub(dutyCycleSubItems, NUMITEMS(dutyCycleSubItems));
    Menu dutyCycleOnSub(dutyCycleOnSubItems, NUMITEMS(dutyCycleOnSubItems), this);
    Menu dutyCycleOffSub(dutyCycleOffSubItems, NUMITEMS(dutyCycleOffSubItems), this);
    menu.addSubMenu(0, &modeSub);
    menu.addSubMenu(1, &targetSub);
    menu.addSubMenu(2, &bandSub);
    menu.addSubMenu(3, &dutyCycleSub);
    dutyCycleSub.addSubMenu(0, &dutyCycleOnSub);
    dutyCycleSub.addSubMenu(1, &dutyCycleOffSub);

    menuDisplay.presentMenu(&menu);
  }
};
