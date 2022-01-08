#include <TFT_22_ILI9225.h>

#define TLX 10
#define TLY 10
#define WIDTH (screenWidth-20)
#define HEIGHT (screenHeight-20)
#define BORDER_WIDTH 3
#define MENU_X (TLX+BORDER_WIDTH)
#define MENU_Y (TLY+BORDER_WIDTH)
#define MENU_WIDTH (WIDTH-2*BORDER_WIDTH)
#define MENU_HEIGHT (HEIGHT-2*BORDER_WIDTH)
#define BACKGROUND_COLOUR COLOR_BLACK
#define MENU_COLOUR COLOR_CYAN
#define NUM_ROWS 6
#define ROW_HEIGHT (MENU_HEIGHT/NUM_ROWS)
#define ROW_X_PAD 3
#define ROW_Y_PAD 5

#define MAX_CALLBACKS 5

#define SELECTED_VALUE_MAX_LEN 16

class MenuCallback {
  public:
  virtual void itemSelected(class Menu *menu, const char *selected) { };
  virtual void subMenuSelected(class Menu *menu, class Menu *subMenu) { };
};

class Menu {
  private:
  
  const char **items;
  unsigned itemCount;
  unsigned activeItem;
  int selectedItem;
  MenuCallback *callbacks[MAX_CALLBACKS];
  Menu **subMenus;
  char selectedValue[SELECTED_VALUE_MAX_LEN + 1];

  TFT_22_ILI9225 *tft;
  unsigned rowCount;
  unsigned drawX;
  unsigned drawY;
  unsigned rowSpace;
  unsigned rowWidth;
  unsigned rowHeight;
  unsigned drawColour;
  unsigned topIndex;

  private:
  void drawItem(unsigned index) {
    const char *selectedText = 0;
    char *text = 0;
    unsigned row = index - topIndex;
    unsigned y = drawY + row * rowSpace;
    
    tft->setBackgroundColor(index==activeItem ? drawColour : COLOR_BLACK);

    if (subMenus[index]) {
      tft->drawTriangle(drawX + rowWidth - rowHeight / 2, y, drawX + rowWidth - rowHeight / 2, y + rowHeight, drawX + rowWidth, y + rowHeight / 2, drawColour);

      selectedText = subMenus[index]->getSelectedValue();
    }

    if (selectedText) {
      text = new char[strlen(items[index]) + strlen(selectedText) + 3];

      sprintf(text, "%s: %s", items[index], selectedText);
    }
    
    tft->drawText(drawX, y, text ? text : items[index], index==activeItem ? COLOR_BLACK : drawColour );

    delete text;
  }

  void postSubMenuCallbacks(Menu *menu, Menu *subMenu) {
    for (int i=0; i<MAX_CALLBACKS && callbacks[i]!=0; i++) {
      callbacks[i]->subMenuSelected(menu, subMenu);
    }
  }

  void postItemCallbacks(Menu *menu, const char *item) {
    for (int i=0; i<MAX_CALLBACKS && callbacks[i]!=0; i++) {
      callbacks[i]->itemSelected(menu, item);
    }
  }
  
  void updateSelectedValueText(void) {
    int length = 0;

    selectedValue[0] = 0;
    
    if (selectedItem >= 0) {
      strncat(selectedValue, items[selectedItem], SELECTED_VALUE_MAX_LEN);
      length += strlen(selectedValue);
    }

    for (int i=0; i<itemCount; i++) {
      if (subMenus[i]) {
        const char *subMenuValue = subMenus[i]->getSelectedValue();

        if (subMenuValue) {
          if (length && (length < SELECTED_VALUE_MAX_LEN)) {
            strcat(selectedValue, "/");
            length++;
          }

          if (length < SELECTED_VALUE_MAX_LEN) {
            strncat(selectedValue, subMenuValue, SELECTED_VALUE_MAX_LEN - length);
            length += strlen(subMenuValue);
          }
        }
      }
    }
  }

  public:
  Menu(const char** items, unsigned itemCount, MenuCallback *callback=0)
  : items(items),
    itemCount(itemCount),
    activeItem(0),
    selectedItem(-1),
    topIndex(0),
    rowCount(UINT_MAX),
    subMenus(new Menu*[itemCount]) {
      memset(subMenus, 0, itemCount * sizeof(Menu*));
      memset(callbacks, 0, MAX_CALLBACKS * sizeof(MenuCallback*));
      selectedValue[0] = 0;

      addCallback(callback);
  }

  virtual ~Menu() {
    delete subMenus;
  }

  void addSubMenu(unsigned itemIndex, Menu *subMenu) {
    subMenus[itemIndex] = subMenu;
  }

  void addCallback(MenuCallback *callback) {
    for (int i=0; i<MAX_CALLBACKS; i++) {
      if (callbacks[i] == callback) {
        break;
      }
      if (callbacks[i] == 0) {
        callbacks[i] = callback;

        break;
      }
    }
  }

  bool adjustViewWindow(void) {
    if (activeItem < topIndex) {
      topIndex = activeItem;
      draw();

      return true;
    }

    if (activeItem - topIndex >= rowCount) {
      topIndex = activeItem - rowCount + 1;
      draw();

      return true;
    }

    return false;
  }

  void upAction(void) {
    if (activeItem > 0) {
      activeItem--;

      if (!adjustViewWindow()) {
        drawItem(activeItem);
        drawItem(activeItem + 1);
      }
    }
  }

  void downAction(void) {
    if (activeItem < itemCount - 1) {
      activeItem++;

      if (!adjustViewWindow()) {
        drawItem(activeItem);
        drawItem(activeItem - 1);
      }
    }
  }

  void selectAction(void) {
    if (subMenus[activeItem]) {
      postSubMenuCallbacks(this, subMenus[activeItem]);
    } else {
      selectedItem = activeItem;

      postItemCallbacks(this, items[selectedItem]);
    }
  }

  const char *getSelectedValue(void) {
    updateSelectedValueText();
    
    return strlen(selectedValue) ? selectedValue : 0;
  }

  int getSelectedIndex(void) {
    return selectedItem;
  }

  void setSelectedIndex(int index) {
    if (index < 0) {
      selectedItem = -1;
      activeItem = 0;
    } else if (index < itemCount) {
      selectedItem = activeItem = index;
    }

    adjustViewWindow();
  }

  void drawInit(TFT_22_ILI9225 &tft, unsigned count, unsigned x, unsigned y, unsigned rowSpace, unsigned width, unsigned height, unsigned colour) {
    this->tft = &tft;
    this->rowCount = count;
    this->drawX = x;
    this->drawY = y;
    this->rowSpace = rowSpace;
    this->rowWidth = width;
    this->rowHeight = height;
    this->drawColour = colour;
  }

  void draw(void) {
    adjustViewWindow();

    tft->fillRectangle(drawX, drawY, drawX + rowWidth, drawY + (rowCount - 1) * rowSpace + rowHeight, BACKGROUND_COLOUR);

    for (int i=0; i<min(rowCount, itemCount); i++) {
      drawItem(topIndex + i);
    }
  }
};

//======================================================
//

class MenuDisplay : public MenuCallback {
  private:
  virtual void itemSelected(class Menu *menu, const char *selected) {
  }
    
  virtual void subMenuSelected(Menu *menu, Menu *subMenu) {
    presentMenu(subMenu);
    
    menu->draw();
  }

  void resetTimeout(void) {
    timeoutCheck = millis();
  }

  bool timeout(void) {
    return millis() - timeoutCheck >= 20000;
  }

  private:
  TFT_22_ILI9225 &tft;
  ButtonController &buttons;
  unsigned long timeoutCheck;
  const unsigned screenWidth;
  const unsigned screenHeight;
  
  public:
  MenuDisplay(TFT_22_ILI9225 &tft, ButtonController &buttons)
  : tft(tft),
    buttons(buttons),
    timeoutCheck(0),
    screenWidth(tft.maxX()),
    screenHeight(tft.maxY()) {
    
  }
  
  void presentMenu(Menu *menu) {
    bool exitMenu = false;
    
    for (int i=0; i<BORDER_WIDTH; i++) {
      tft.drawRectangle(TLX+i, TLY+i, TLX+WIDTH-i, TLY+HEIGHT-i, MENU_COLOUR);
    }
    tft.fillRectangle(MENU_X, MENU_Y, MENU_X+MENU_WIDTH, MENU_Y+MENU_HEIGHT, BACKGROUND_COLOUR);
    tft.setFont(Terminal11x16);
    
    menu->addCallback(this);
    menu->drawInit(tft, NUM_ROWS, MENU_X+ROW_X_PAD, MENU_Y+ROW_Y_PAD, ROW_HEIGHT, MENU_WIDTH-2*ROW_X_PAD, tft.getFont().height, MENU_COLOUR);
    menu->draw();

    resetTimeout();

    while (!exitMenu && !timeout()) {
      if (buttons.buttonPressed(ButtonUp)) {
        menu->upAction();

        resetTimeout();
      }

      if (buttons.buttonPressed(ButtonDown)) {
        menu->downAction();

        resetTimeout();
      }

      if (buttons.buttonPressed(ButtonSelect)) {
        menu->selectAction();

        if (menu->getSelectedIndex() >= 0) {
          exitMenu = true;
        }

        resetTimeout();
      }

      if (buttons.buttonPressed(ButtonBack)) {
        exitMenu = true;
      }

      delay(50);
    }
  }
};
