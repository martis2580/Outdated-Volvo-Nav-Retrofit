#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

#include "config.h"
#include "state_machine.h"
#include "can_handler.h"

static MCP_CAN CAN(CAN_CS_PIN);
static const bool CAN_USE_LISTEN_ONLY = true;

void initCan()
{
  while (CAN.begin(MCP_STDEXT, CAN_125KBPS, MCP_16MHZ) != CAN_OK)
  {
    delay(500);
  }

  if (CAN_USE_LISTEN_ONLY)
  {
    CAN.setMode(MCP_LISTENONLY);
  }
  else
  {
    CAN.setMode(MCP_NORMAL);
  }
}

void processCan()
{
  while (CAN_MSGAVAIL == CAN.checkReceive())
  {
    unsigned long now = millis();
    uint32_t id = 0;
    uint8_t len = 0;
    uint8_t buf[8];
    uint8_t ext = 0;
    CAN.readMsgBuf(&id, &ext, &len, buf);
    noteCanFrameReceived(now);

    if (id == IGNITION_CAN_ID && len > IGNITION_BYTE_INDEX)
    {
      bool ignitionOn = (buf[IGNITION_BYTE_INDEX] & IGNITION_ON_MASK) == IGNITION_ON_VALUE;
      processIgnitionFrame(ignitionOn, now);
    }
  }
}
