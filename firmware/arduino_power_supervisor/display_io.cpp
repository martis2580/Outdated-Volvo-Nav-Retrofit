#include <Arduino.h>

#include "config.h"
#include "state_machine.h"
#include "display_io.h"

static bool screenVisible = false;

void initDisplayIo()
{
  pinMode(DC_DC_RELAY_PIN, OUTPUT);
  pinMode(PI_SHUTDOWN_RELAY_PIN, OUTPUT);
  setDcDcRelay(false);
  setPiShutdownRelay(false);
  screenVisible = false;
}

void setDcDcRelay(bool on)
{
  digitalWrite(DC_DC_RELAY_PIN, on ? HIGH : LOW);
}

void setPiShutdownRelay(bool on)
{
  digitalWrite(PI_SHUTDOWN_RELAY_PIN, on ? HIGH : LOW);
}

void setScreenVisible(bool visible)
{
  screenVisible = visible;
}

void sendScreenCommand()
{
  Serial.write((uint8_t)(screenVisible ? SCREEN_PAL : SCREEN_OFF));
  delay(50);
  Serial.write((uint8_t)signal_brightnes);
  delay(50);
  Serial.write((uint8_t)0x83);
  delay(50);
}
