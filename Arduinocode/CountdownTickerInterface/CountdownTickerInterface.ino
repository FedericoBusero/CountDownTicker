#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
// NodeMCU: SDA=D2=GPIO4, SCL=D1=GPIO5
//#define LCD_I2C_ADDRESS 0x3F
#define LCD_I2C_ADDRESS 0x38

#else
#define LCD_I2C_ADDRESS 0x27

#endif

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2); // set the LCD address to 0x3F for a 16 chars and 2 line display

#include "Button.h"
/*
  TODO:
  - don't use String class
*/

#define SERIAL_RASPI Serial

enum
{
  BUTTON_RESET = 1,
  BUTTON_PAUSEPLAY,
  BUTTON_STOP,
  BUTTON_STOP_LONG,
  BUTTON_MINUTES_PLUS,
  BUTTON_MINUTES_MIN,
  BUTTON_SECONDS_PLUS,
  BUTTON_SECONDS_MIN,
};

enum
{
  MODE_BOOTING = 0,
  MODE_RUNNING,
  MODE_PAUSED,
  MODE_STOPPED,
  MODE_QUIT,
};

#ifdef ESP8266
ButtonRepeat buttonMinutesPlus(D3, BUTTON_MINUTES_PLUS);
ButtonRepeat buttonMinutesMin (D4, BUTTON_MINUTES_MIN);
ButtonRepeat buttonSecondsPlus(D6, BUTTON_SECONDS_PLUS);
ButtonRepeat buttonSecondsMin (D7, BUTTON_SECONDS_MIN);
Button       buttonReset      (D5, BUTTON_RESET);
Button       buttonPausePlay  (D0, BUTTON_PAUSEPLAY);
Button       buttonStop       (10, BUTTON_STOP); // GPIO10
//ButtonLong   buttonStop       (10, BUTTON_STOP, BUTTON_STOP_LONG); // GPIO10
#else
ButtonRepeat buttonMinutesPlus(2, BUTTON_MINUTES_PLUS);
ButtonRepeat buttonMinutesMin (3, BUTTON_MINUTES_MIN);
ButtonRepeat buttonSecondsPlus(4, BUTTON_SECONDS_PLUS);
ButtonRepeat buttonSecondsMin (5, BUTTON_SECONDS_MIN);
Button       buttonReset      (6, BUTTON_RESET);
Button       buttonPausePlay  (7, BUTTON_PAUSEPLAY);
ButtonLong   buttonStop       (8, BUTTON_STOP, BUTTON_STOP_LONG); 
#endif

String inputString = "";         // a String to hold incoming data

int nexttimer_hour = 0;
int nexttimer_minutes = 0;
int nexttimer_seconds = 0;
int currentmode = MODE_BOOTING;

void nexttimer_init()
{
  nexttimer_hour = 0;
  nexttimer_minutes = 0;
  nexttimer_seconds = 0;
}

void nexttimer_minutes_plus()
{
  nexttimer_minutes += nexttimer_hour * 60;
  nexttimer_hour = 0;

  if (nexttimer_minutes < 9 * 60 + 59) // max 9u59 min (1 digit hour)
  {
    ++nexttimer_minutes;
  }

  nexttimer_hour = nexttimer_minutes / 60;
  nexttimer_minutes = nexttimer_minutes % 60;
}

void nexttimer_minutes_min()
{
  nexttimer_minutes += nexttimer_hour * 60;
  nexttimer_hour = 0;

  if (nexttimer_minutes > 0)
  {
    --nexttimer_minutes;
  }

  nexttimer_hour = nexttimer_minutes / 60;
  nexttimer_minutes = nexttimer_minutes % 60;
}

void nexttimer_seconds_plus()
{
  ++nexttimer_seconds;
  if (nexttimer_seconds == 60)
  {
    nexttimer_seconds = 0;
    nexttimer_minutes_plus();
  }
}

void nexttimer_seconds_min()
{
  --nexttimer_seconds;
  if (nexttimer_seconds == -1)
  {
    nexttimer_seconds = 59;
    nexttimer_minutes_min();
  }
}

void ticker_StartCountdown(int _hour, int _minute, int _second) {
  SERIAL_RASPI.print(F("StartCountdown "));
  SERIAL_RASPI.print(_hour);
  SERIAL_RASPI.print(' ');
  SERIAL_RASPI.print(_minute);
  SERIAL_RASPI.print(' ');
  SERIAL_RASPI.print(_second);
  SERIAL_RASPI.print(F("\r\n"));
}

void ticker_ResetCountdown(int _hour, int _minute, int _second) {
  SERIAL_RASPI.print(F("ResetCountdown "));
  SERIAL_RASPI.print(_hour);
  SERIAL_RASPI.print(' ');
  SERIAL_RASPI.print(_minute);
  SERIAL_RASPI.print(' ');
  SERIAL_RASPI.print(_second);
  SERIAL_RASPI.print(F("\r\n"));
}


void ticker_pause() {
  SERIAL_RASPI.print(F("Pause"));
  SERIAL_RASPI.print(F("\r\n"));
}

void ticker_stop() {
  SERIAL_RASPI.print(F("Stop"));
  SERIAL_RASPI.print(F("\r\n"));
}

void ticker_quit() {
  SERIAL_RASPI.print(F("Quit"));
  SERIAL_RASPI.print(F("\r\n"));
}

void ticker_ping() {
  SERIAL_RASPI.print(F("Ping"));
  SERIAL_RASPI.print(F("\r\n"));
}

void updatedisplay()
{
  String str = "Next: ";
  lcd.setCursor(0, 0);
  if (nexttimer_hour)
  {
    str += (char)('0' + (nexttimer_hour % 10));
    str += ':';
  }
  else
  {
    str += "  ";
  }
  str += (char)('0' + (nexttimer_minutes / 10));
  str += (char)('0' + (nexttimer_minutes % 10));
  str += ':';
  str += (char)('0' + (nexttimer_seconds / 10));
  str += (char)('0' + (nexttimer_seconds % 10));
  str += "  ";
  lcd.print(str);
}


void onButtonPressed(int id)
{
  switch (id)
  {
    case BUTTON_RESET:
      if (currentmode != MODE_BOOTING)
      {
        ticker_ResetCountdown(nexttimer_hour, nexttimer_minutes, nexttimer_seconds);
      }
      break;

    case BUTTON_PAUSEPLAY:
      switch (currentmode)
      {
        case MODE_BOOTING:
          break;

        case MODE_PAUSED:
        case MODE_RUNNING:
          ticker_pause();
          break;

        case MODE_STOPPED:
          if ((nexttimer_hour > 0) || (nexttimer_minutes > 0) || (nexttimer_seconds > 0))
          {
            ticker_StartCountdown(nexttimer_hour, nexttimer_minutes, nexttimer_seconds);
          }
          break;

      }
      break;

    case BUTTON_STOP:
      if (currentmode != MODE_BOOTING)
      {
        ticker_stop();
      }
      break;

    case BUTTON_STOP_LONG:
      if (currentmode != MODE_BOOTING)
      {
        ticker_quit();
      }
      break;

    case BUTTON_MINUTES_PLUS:
      nexttimer_minutes_plus();
      updatedisplay();
      break;

    case BUTTON_MINUTES_MIN:
      nexttimer_minutes_min();
      updatedisplay();
      break;

    case BUTTON_SECONDS_PLUS:
      nexttimer_seconds_plus();
      updatedisplay();
      break;

    case BUTTON_SECONDS_MIN:
      nexttimer_seconds_min();
      updatedisplay();
      break;
  }
}

void onButtonReleased(int id)
{

}

void setup() {
  SERIAL_RASPI.begin(115200);
  inputString.reserve(200);
#ifdef ESP8266
  WiFi.mode(WIFI_OFF);
#endif
  nexttimer_init();

  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  updatedisplay();
  lcd.setCursor(0, 1);
  lcd.print("Booting ...     ");
  ticker_ping();
}

void loop() {
  while (SERIAL_RASPI.available()) {
    char inChar = (char)SERIAL_RASPI.read();
    inputString += inChar;
    if (inChar == '\n') {
      inputString.trim();
      int pos = inputString.indexOf(',');
      if (pos >= 0)
      {
        if (inputString.length() >= pos + 2)
        {
          char mode_char = inputString.charAt(pos + 1);
          if ((mode_char >= '0') && (mode_char <= '5'))
          {
            currentmode = mode_char - '0';
            if (pos == 0)
            {
              inputString = "";
            }
            else
            {
              inputString = inputString.substring(0, pos);
            }

            switch (currentmode)
            {
              case MODE_BOOTING:
                break;

              case MODE_RUNNING:
                break;

              case MODE_PAUSED:
                break;

              case MODE_STOPPED:
                updatedisplay();
                break;

              case MODE_QUIT:
                lcd.setCursor(0, 0);
                lcd.print("Shutdown        ");
                lcd.setCursor(0, 1);
                lcd.print("                ");
                break;
            }
          }
        }
      }
      lcd.setCursor(0, 1);
      lcd.print(inputString + "                ");
      // clear the string:
      inputString = "";
    }
  }

  buttonPausePlay.checkStatus();
  buttonMinutesPlus.checkStatus();
  buttonMinutesMin.checkStatus();
  buttonReset.checkStatus();
  buttonSecondsPlus.checkStatus();
  buttonSecondsMin.checkStatus();
  buttonStop.checkStatus();
}
