#pragma once

#include <Arduino.h>

enum ControlState
{
  STATE_WAIT_FOR_IGNITION = 0,
  STATE_RUNNING,
  STATE_CAN_SLEEP,
  STATE_PI_SHUTDOWN_PULSE_1_ON,
  STATE_PI_SHUTDOWN_PULSE_1_GAP,
  STATE_PI_SHUTDOWN_PULSE_2_ON,
  STATE_PI_SHUTDOWN_PULSE_2_GAP,
  STATE_PI_POWERDOWN_WAIT
};

extern char signal_brightnes;
extern ControlState controlState;

void resetToWaitingState(bool keepDcDcOn);
inline void resetToWaitingState()
{
  resetToWaitingState(false);
}
void noteCanFrameReceived(unsigned long now);
void processIgnitionFrame(bool ignitionOn, unsigned long now);
void updateControlState();
