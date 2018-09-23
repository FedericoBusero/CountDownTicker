#ifndef _BUTTON_H_
#define _BUTTON_H_

#define DEBOUNCE_DELAY 30
#define LONGPRESS_TIME 4000L
#define REPEAT_START 1000L
#define REPEAT_TIME 100L

extern void onButtonPressed(int id);
extern void onButtonReleased(int id);


void button_pin_setup(int buttonpin)
{
#ifdef ESP8266
  if ((buttonpin == 1) || (buttonpin == 3)) // GPIO1, GPIO3 : RX & TX ESP-01
  {
    pinMode (buttonpin, FUNCTION_3);
  }
  if (buttonpin == 15) // GPIO15 (D8 on NodeMCU, daar zit typisch externe pulldown op)
  {
    pinMode(buttonpin, INPUT);
  }
  else if (buttonpin == 16) // GPIO16 (D0 on NodeMCU)
  {
    pinMode(buttonpin, INPUT_PULLDOWN_16);
  }
  else
  {
    pinMode(buttonpin, INPUT_PULLUP);
  }
#else
  pinMode(buttonpin, INPUT_PULLUP);
#endif

}

bool button_pin_read(int buttonpin)
{
#ifdef ESP8266
  if ((buttonpin == 15) || (buttonpin == 16)) // GPIO15 (D8 on NodeMCU), GPIO16 (D0 on NodeMCU)
  {
    return (digitalRead(buttonpin));
  }
  else
  {
    return (!digitalRead(buttonpin));
  }
#else
  return (!digitalRead(buttonpin));
#endif

}


class Button
{
  public:
    Button(int _buttonpin, int _id)
      : id(_id), buttonstate(0), buttonpin(_buttonpin)
    {
      button_pin_setup(_buttonpin);
    }

    void checkStatus()
    {
      int newstate = button_pin_read(buttonpin);

      if (newstate != buttonstate)
      {
        delay(DEBOUNCE_DELAY);
        if (newstate)
        {
          onButtonPressed(id);
        }
        else
        {
          onButtonReleased(id);
        }
      }
      buttonstate = newstate;
    }

  private:
    int buttonpin;
    int id;
    int buttonstate;
};


class ButtonLong
{
  public:
    ButtonLong(int _buttonpin, int _id, int _idlong)
      : id(_id), buttonstate(0), buttonpin(_buttonpin), idlong(_idlong), timerstate(0), nexteventtime(0)
    {
      button_pin_setup(_buttonpin);
    }

    void checkStatus()
    {
      int newstate = button_pin_read(buttonpin);

      if (newstate != buttonstate)
      {
        delay(DEBOUNCE_DELAY);

        if (newstate)
        {
          nexteventtime = millis() + LONGPRESS_TIME;
          onButtonPressed(id);
          timerstate = 1;
        }
        else
        {
          switch (timerstate)
          {
            case 1:
              onButtonReleased(id);
              break;
            case 2:
              onButtonReleased(idlong);
              break;
          }
          timerstate = 0;
          nexteventtime = 0;
        }
      }
      else
      {
        if (newstate && (1 == timerstate) && (millis() > nexteventtime))
        {
          onButtonReleased(id);
          onButtonPressed(idlong);
          timerstate = 2;
        }
      }
      buttonstate = newstate;
    }

  private:
    int buttonpin;
    int id;
    int idlong;
    int buttonstate;
    int timerstate;
    unsigned long nexteventtime;
};


class ButtonRepeat
{
  public:
    ButtonRepeat(int _buttonpin, int _id)
      : id(_id), buttonstate(0), buttonpin(_buttonpin), timerstate(0), nexteventtime(0)
    {
      button_pin_setup(_buttonpin);
    }

    void checkStatus()
    {
      int newstate = button_pin_read(buttonpin);

      if (newstate != buttonstate)
      {
        delay(DEBOUNCE_DELAY);

        if (newstate)
        {
          nexteventtime = millis() + REPEAT_START;
          onButtonPressed(id);
          timerstate = 1;
        }
        else
        {
          onButtonReleased(id);
          timerstate = 0;
          nexteventtime = 0;
        }
      }
      else
      {
        if (newstate && (millis() > nexteventtime))
        {
          onButtonReleased(id);
          onButtonPressed(id);
          timerstate = 2;
          nexteventtime+=REPEAT_TIME;
        }
      }
      buttonstate = newstate;
    }

  private:
    int buttonpin;
    int id;
    int buttonstate;
    int timerstate;
    unsigned long nexteventtime;
};


#endif

