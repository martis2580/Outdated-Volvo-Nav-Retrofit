#pragma once

#include <Arduino.h>

const char SCREEN_PAL = 0x45;
const char SCREEN_OFF = 0x46;
const char screen_brightnes[] = {0x20, 0x61, 0x62, 0x23, 0x64, 0x25, 0x26, 0x67, 0x68, 0x29, 0x2A, 0x2C, 0x6B, 0x6D, 0x6E, 0x2F};

const int CAN_CS_PIN = 10;
const int CAN_INT_PIN = 2;
const int DC_DC_RELAY_PIN = 9;
const int PI_SHUTDOWN_RELAY_PIN = 8;

const uint32_t IGNITION_CAN_ID = 0x217FFC;
const uint8_t IGNITION_BYTE_INDEX = 5;
const uint8_t IGNITION_ON_MASK = 0xF8;
const uint8_t IGNITION_ON_VALUE = 0x58;
const uint8_t IGNITION_ON_ALT_VALUE_1 = 0x98;               // cruise-control active variant on the same CAN frame
const uint8_t IGNITION_ON_ALT_VALUE_2 = 0xD8;               // speed +/- variant on the same CAN frame

const unsigned long IGNITION_ON_STABLE_MS = 10000UL;
const unsigned long IGNITION_ON_GAP_RESET_MS = 3000UL;      // tolerate normal CAN spacing, but reset after a real interruption
const unsigned long IGNITION_LOSS_MS = 5000UL;
const unsigned long SCREEN_ON_DELAY_MS = 7000UL;
const unsigned long CRANK_GRACE_MS = 15000UL;               // block relay-9 power cut during the first seconds after key-on
const unsigned long CAN_IDLE_SLEEP_MS = 300000UL;            // 5 minutes without any CAN traffic enters sleep mode
const unsigned long PI_SHUTDOWN_PULSE_MS = 250UL;
const unsigned long PI_SHUTDOWN_GAP_MS = 2000UL;
const unsigned long PI_POWERDOWN_WAIT_MS = 60000UL;
