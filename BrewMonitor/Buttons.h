typedef enum {
  ButtonAny,
  ButtonUp,
  ButtonDown,
  ButtonSelect,
  ButtonBack
} Buttons;

class ButtonController {
  private:
  int upPin;
  int downPin;
  int selPin;
  int backPin;

  private:
  bool checkButton(int pin) {
    if (digitalRead(pin) != LOW) {
      return false;
    }

    do {
      delay(25);
    } while (digitalRead(pin) == LOW);

    return true;
  }
  
  public:
  void init(int up, int down, int sel, int back) {
    pinMode(up, INPUT_PULLUP);
    upPin = up;
    pinMode(down, INPUT_PULLUP);
    downPin = down;
    pinMode(sel, INPUT_PULLUP);
    selPin = sel;
    pinMode(back, INPUT_PULLUP);
    backPin = back;
  }
  
  bool buttonPressed(Buttons button) {
    switch(button) {
      case ButtonUp:
        return checkButton(upPin);
        break;
      case ButtonDown:
        return checkButton(downPin);
        break;
      case ButtonSelect:
        return checkButton(selPin);
        break;
      case ButtonBack:
        return checkButton(backPin);
        break;
      case ButtonAny:
        return checkButton(upPin) || checkButton(downPin) || checkButton(selPin) || checkButton(backPin);
        break;
    }
    
    return false;
  }
};
