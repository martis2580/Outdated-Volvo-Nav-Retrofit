#pragma once

#include <Arduino.h>

static inline unsigned long elapsedSince(unsigned long startMillis, unsigned long nowMillis)
{
  return nowMillis - startMillis;
}

static inline bool hasElapsed(unsigned long startMillis, unsigned long intervalMillis, unsigned long nowMillis)
{
  return elapsedSince(startMillis, nowMillis) >= intervalMillis;
}

static inline bool hasElapsedStrict(unsigned long startMillis, unsigned long intervalMillis, unsigned long nowMillis)
{
  return elapsedSince(startMillis, nowMillis) > intervalMillis;
}
