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

#define LCD_NUMCHARS 16
#define MAX_BUFFERSIZE 21 // minimum LCD_NUMCHARS+5

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_NUMCHARS, 2); // set the LCD address to 0x3F for a 16 chars and 2 line display

#include "Button.h"

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
ButtonRepeat buttonMinutesPlus(D3, BUTTON_MINUTES_PLUS); // GPIO0
ButtonRepeat buttonMinutesMin (D4, BUTTON_MINUTES_MIN);  // GPIO2
ButtonRepeat buttonSecondsPlus(D6, BUTTON_SECONDS_PLUS); // GPIO12
ButtonRepeat buttonSecondsMin (D7, BUTTON_SECONDS_MIN);  // GPIO13
Button       buttonReset      (D5, BUTTON_RESET);        // GPIO14
Button       buttonPausePlay  (D0, BUTTON_PAUSEPLAY);    // GPIO16
Button       buttonStop       (10, BUTTON_STOP);         // GPIO10
//ButtonLong   buttonStop       (10, BUTTON_STOP, BUTTON_STOP_LONG); // GPIO10
#else
ButtonRepeat buttonMinutesPlus(3, BUTTON_MINUTES_PLUS);
ButtonRepeat buttonMinutesMin (2, BUTTON_MINUTES_MIN);
ButtonRepeat buttonSecondsPlus(5, BUTTON_SECONDS_PLUS);
ButtonRepeat buttonSecondsMin (4, BUTTON_SECONDS_MIN);
Button       buttonReset      (6, BUTTON_RESET);
Button       buttonPausePlay  (7, BUTTON_PAUSEPLAY);
ButtonLong   buttonStop       (8, BUTTON_STOP, BUTTON_STOP_LONG); 
#endif

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
    if ((nexttimer_minutes == 0) && (nexttimer_hour == 0))
    {
      nexttimer_seconds = 0;
    }
    else
    {
      nexttimer_seconds = 59;
      nexttimer_minutes_min();
    }
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
  SERIAL_RASPI.print(F("\r\n"));
  SERIAL_RASPI.print(F("Ping"));
  SERIAL_RASPI.print(F("\r\n"));
}

void updatedisplay_nexttimer()
{
  char display[17]; // LCD_NUMCHARS+1

  strcpy(display, "Next:     :     ");
  if (nexttimer_hour)
  {
    display[6] = (char)('0' + (nexttimer_hour % 10));
    display[7] = ':';
  }
  display[8] = (char)('0' + (nexttimer_minutes / 10));
  display[9] = (char)('0' + (nexttimer_minutes % 10));
  display[11] = (char)('0' + (nexttimer_seconds / 10));
  display[12] = (char)('0' + (nexttimer_seconds % 10));

  lcd.setCursor(0, 0);
  lcd.print(display);
}

void lcd_print_filled(int c, int r, const char *str)
{
  int i;
  char screenbuf[LCD_NUMCHARS + 1];

  for (i = 0; i < LCD_NUMCHARS && str[i] != '\0'; i++)
  {
    screenbuf[i] = str[i];
  }
  for ( ; i < LCD_NUMCHARS; i++)
  {
    screenbuf[i] = ' ';
  }
  screenbuf[LCD_NUMCHARS] = '\0';

  lcd.setCursor(c, r);
  lcd.print(screenbuf);
}

void ticker_updateStatus(const char *statusstring, int newmode) {
  currentmode = newmode;
  switch (currentmode)
  {
    case MODE_BOOTING:
      break;

    case MODE_RUNNING:
    case MODE_PAUSED:
    case MODE_STOPPED:
      updatedisplay_nexttimer();
      lcd_print_filled(0, 1, statusstring);
      break;

    case MODE_QUIT:
      lcd_print_filled(0, 0, "Shutdown");
      lcd_print_filled(0, 1, "");
      break;
  }
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
      updatedisplay_nexttimer();
      break;

    case BUTTON_MINUTES_MIN:
      nexttimer_minutes_min();
      updatedisplay_nexttimer();
      break;

    case BUTTON_SECONDS_PLUS:
      nexttimer_seconds_plus();
      updatedisplay_nexttimer();
      break;

    case BUTTON_SECONDS_MIN:
      nexttimer_seconds_min();
      updatedisplay_nexttimer();
      break;
  }
}

void onButtonReleased(int id)
{

}

void setup() {
  SERIAL_RASPI.begin(115200);
#ifdef ESP8266
  WiFi.mode(WIFI_OFF);
#endif
  nexttimer_init();

  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  updatedisplay_nexttimer();
  lcd.setCursor(0, 1);
  lcd_print_filled(0, 1, "Booting ...");
  ticker_ping();
}

void processSerial()
{
  static int buffersize = 0;
  static char serialbuffer[MAX_BUFFERSIZE + 1];

  while (SERIAL_RASPI.available()) {
    char inChar = (char)SERIAL_RASPI.read();
    serialbuffer[buffersize++] = inChar;
    serialbuffer[buffersize] = '\0';
    if (buffersize >= MAX_BUFFERSIZE)
    {
      // flush all bytes
      while (SERIAL_RASPI.available()) {
        SERIAL_RASPI.read();
      }
      buffersize = 0;
    }
    else
    {
      if (inChar == '\n')  {
        char *pos = strrchr(serialbuffer, ',');
        if (*pos == ',')
        {
          *pos = '\0';
          ++pos;
          int newmode = atoi(pos);
          if ((newmode >= 1) && (newmode <= 4))
          {
            ticker_updateStatus(serialbuffer, newmode);
          }
        }
        buffersize = 0;
      }
    }
  }
}

void loop() {
  // Process incoming bytes on RASPI_SERIAL
  processSerial();

  // Check button states
  buttonPausePlay.checkStatus();
  buttonMinutesPlus.checkStatus();
  buttonMinutesMin.checkStatus();
  buttonReset.checkStatus();
  buttonSecondsPlus.checkStatus();
  buttonSecondsMin.checkStatus();
  buttonStop.checkStatus();
}
