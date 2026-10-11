#include <Arduino.h>
#if defined(ARDUINO_ARCH_AVR)
#include <avr/io.h>
#include <avr/sleep.h>
#endif

#include "config.h"
#include "display_io.h"
#include "can_handler.h"
#include "state_machine.h"

#if defined(ARDUINO_ARCH_AVR)
volatile bool canWakeInterruptSeen = false;

void wakeFromCanInterrupt()
{
  canWakeInterruptSeen = true;
}

static void sleepUntilCanInterrupt()
{
  canWakeInterruptSeen = false;
  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), wakeFromCanInterrupt, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(CAN_INT_PIN));
  resetToWaitingState();
}
#else
static void sleepUntilCanInterrupt()
{
  delay(10);
  resetToWaitingState();
}
#endif

void setup()
{
  pinMode(CAN_INT_PIN, INPUT_PULLUP);
  Serial.begin(2400);

  initDisplayIo();
  initCan();
  resetToWaitingState(false);
  signal_brightnes = screen_brightnes[15];
}
void loop()
{
  processCan();
  updateControlState();
  if (controlState == STATE_CAN_SLEEP)
  {
    sleepUntilCanInterrupt();
    return;
  }

  sendScreenCommand();
}
