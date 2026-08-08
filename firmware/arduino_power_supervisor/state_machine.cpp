#include <Arduino.h>

#include "config.h"
#include "display_io.h"
#include "state_machine.h"
#include "time_utils.h"

char signal_brightnes = 0;
ControlState controlState = STATE_WAIT_FOR_IGNITION;

unsigned long ignitionOnSinceMillis = 0;
unsigned long ignitionOnLastSeenMillis = 0;
bool ignitionOnPending = false;
unsigned long ignitionOffSinceMillis = 0;
bool ignitionOffPending = false;

unsigned long runningSinceMillis = 0;
unsigned long crankGraceStartedMillis = 0;
bool runningActive = false;
bool dcDcRelayOn = false;
bool screenOnPending = false;
bool dcDcCutArmed = false;

unsigned long phaseStartedMillis = 0;
unsigned long shutdownStartedMillis = 0;
unsigned long lastCanFrameMillis = 0;

static void syncDisplayOutputs()
{
  setScreenVisible(controlState == STATE_RUNNING && !screenOnPending);
}

static void armDcDcCut()
{
  dcDcCutArmed = true;
}

static void disarmDcDcCut()
{
  dcDcCutArmed = false;
}

static void resetIgnitionStartup()
{
  ignitionOnPending = false;
  ignitionOnSinceMillis = 0;
  ignitionOnLastSeenMillis = 0;
  ignitionOffPending = false;
  ignitionOffSinceMillis = 0;
}

static void enterCanSleepMode(unsigned long now)
{
  resetToWaitingState(false);
  controlState = STATE_CAN_SLEEP;
  lastCanFrameMillis = now;
}

static void enterRunning(unsigned long now)
{
  controlState = STATE_RUNNING;
  runningActive = true;
  dcDcRelayOn = true;
  disarmDcDcCut();
  runningSinceMillis = now;
  crankGraceStartedMillis = now;
  screenOnPending = true;
  ignitionOnPending = false;
  ignitionOnSinceMillis = 0;
  ignitionOffPending = false;
  ignitionOffSinceMillis = 0;
  setDcDcRelay(true);
}

static void startPiShutdownSequence(unsigned long now)
{
  controlState = STATE_PI_SHUTDOWN_PULSE_1_ON;
  runningActive = false;
  screenOnPending = false;
  setScreenVisible(false);
  setPiShutdownRelay(true);
  disarmDcDcCut();
  shutdownStartedMillis = now;
  phaseStartedMillis = now;
}

static void abortShutdownAndRestart(unsigned long now)
{
  setPiShutdownRelay(false);
  controlState = STATE_WAIT_FOR_IGNITION;
  runningActive = false;
  dcDcRelayOn = true;
  screenOnPending = false;
  disarmDcDcCut();
  ignitionOnPending = true;
  ignitionOnSinceMillis = now;
  ignitionOnLastSeenMillis = now;
  ignitionOffPending = false;
  ignitionOffSinceMillis = 0;
  runningSinceMillis = 0;
  crankGraceStartedMillis = 0;
  phaseStartedMillis = 0;
  shutdownStartedMillis = 0;
  setDcDcRelay(true);
}

void resetToWaitingState(bool keepDcDcOn)
{
  controlState = STATE_WAIT_FOR_IGNITION;
  runningActive = false;
  dcDcRelayOn = keepDcDcOn || !dcDcCutArmed;
  screenOnPending = false;
  ignitionOnPending = false;
  ignitionOnSinceMillis = 0;
  ignitionOnLastSeenMillis = 0;
  ignitionOffPending = false;
  ignitionOffSinceMillis = 0;
  runningSinceMillis = 0;
  crankGraceStartedMillis = 0;
  phaseStartedMillis = 0;
  shutdownStartedMillis = 0;
  disarmDcDcCut();
  lastCanFrameMillis = millis();
  setDcDcRelay(dcDcRelayOn);
  setPiShutdownRelay(false);
}

void noteCanFrameReceived(unsigned long now)
{
  lastCanFrameMillis = now;
}

void processIgnitionFrame(bool ignitionOn, unsigned long now)
{
  if (ignitionOn)
  {
    if (controlState != STATE_RUNNING && controlState != STATE_WAIT_FOR_IGNITION)
    {
      abortShutdownAndRestart(now);
    }

    if (!ignitionOnPending || ignitionOnLastSeenMillis == 0 || hasElapsedStrict(ignitionOnLastSeenMillis, IGNITION_ON_GAP_RESET_MS, now))
    {
      ignitionOnPending = true;
      ignitionOnSinceMillis = now;
    }

    ignitionOnLastSeenMillis = now;
    ignitionOffPending = false;
    ignitionOffSinceMillis = 0;
  }
  else
  {
    ignitionOnPending = false;
    ignitionOnSinceMillis = 0;
    ignitionOnLastSeenMillis = 0;
    ignitionOffPending = true;
    if (!screenOnPending && ignitionOffSinceMillis == 0)
    {
      ignitionOffSinceMillis = now;
    }
  }
}

void updateControlState()
{
  unsigned long now = millis();
  bool crankGraceActive = (controlState == STATE_RUNNING) && !hasElapsed(crankGraceStartedMillis, CRANK_GRACE_MS, now);

  bool ignitionActivityInProgress = ignitionOnPending || ignitionOffPending || runningActive || controlState == STATE_RUNNING;
  bool mayEnterCanSleep = controlState == STATE_WAIT_FOR_IGNITION && !ignitionActivityInProgress;

  if (mayEnterCanSleep && lastCanFrameMillis != 0 && hasElapsed(lastCanFrameMillis, CAN_IDLE_SLEEP_MS, now))
  {
    enterCanSleepMode(now);
  }

  if (controlState == STATE_CAN_SLEEP)
  {
    syncDisplayOutputs();
    return;
  }

  if (ignitionOnPending && ignitionOnLastSeenMillis != 0 && hasElapsedStrict(ignitionOnLastSeenMillis, IGNITION_ON_GAP_RESET_MS, now))
  {
    resetIgnitionStartup();
  }

  switch (controlState)
  {
    case STATE_WAIT_FOR_IGNITION:
      if (ignitionOnPending && hasElapsed(ignitionOnSinceMillis, IGNITION_ON_STABLE_MS, now))
      {
        enterRunning(now);
      }
      break;

    case STATE_RUNNING:
      if (screenOnPending && hasElapsed(runningSinceMillis, SCREEN_ON_DELAY_MS, now))
      {
        screenOnPending = false;
        if (ignitionOffPending && ignitionOffSinceMillis == 0 && !crankGraceActive)
        {
          ignitionOffSinceMillis = now;
        }
        break;
      }

      if (!screenOnPending && ignitionOffPending && !crankGraceActive)
      {
        if (ignitionOffSinceMillis == 0)
        {
          ignitionOffSinceMillis = now;
        }

        if (hasElapsed(ignitionOffSinceMillis, IGNITION_LOSS_MS, now))
        {
          startPiShutdownSequence(now);
        }
      }
      break;

    case STATE_PI_SHUTDOWN_PULSE_1_ON:
      if (hasElapsed(phaseStartedMillis, PI_SHUTDOWN_PULSE_MS, now))
      {
        setPiShutdownRelay(false);
        controlState = STATE_PI_SHUTDOWN_PULSE_1_GAP;
        phaseStartedMillis = now;
      }
      break;

    case STATE_PI_SHUTDOWN_PULSE_1_GAP:
      if (hasElapsed(phaseStartedMillis, PI_SHUTDOWN_GAP_MS, now))
      {
        setPiShutdownRelay(true);
        controlState = STATE_PI_SHUTDOWN_PULSE_2_ON;
        phaseStartedMillis = now;
      }
      break;

    case STATE_PI_SHUTDOWN_PULSE_2_ON:
      if (hasElapsed(phaseStartedMillis, PI_SHUTDOWN_PULSE_MS, now))
      {
        setPiShutdownRelay(false);
        controlState = STATE_PI_SHUTDOWN_PULSE_2_GAP;
        phaseStartedMillis = now;
      }
      break;

    case STATE_PI_SHUTDOWN_PULSE_2_GAP:
      if (hasElapsed(phaseStartedMillis, PI_SHUTDOWN_GAP_MS, now))
      {
        armDcDcCut();
        controlState = STATE_PI_POWERDOWN_WAIT;
        phaseStartedMillis = now;
      }
      break;

    case STATE_PI_POWERDOWN_WAIT:
      if (hasElapsed(shutdownStartedMillis, PI_POWERDOWN_WAIT_MS, now))
      {
        resetToWaitingState(false);
      }
      break;

    case STATE_CAN_SLEEP:
      break;
  }

  syncDisplayOutputs();
}
