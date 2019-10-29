#include "arduino.h"
#include <lmic.h>
#include <hal/hal.h>
#include <limits.h>
#include "message.h"
#include "debug.h"
#include "relay.h"
#include "lora.h"
#include "automation.h"
#include "streetlight-control.h"
#include "config/local.h"

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = __APPEUI;
static const u1_t PROGMEM DEVEUI[8] = __DEVEUI;

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = __APPKEY;

void os_getArtEui(u1_t *buf)
{
    memcpy_P(buf, APPEUI, 8);
}

void os_getDevEui(u1_t *buf)
{
    memcpy_P(buf, DEVEUI, 8);
}

void os_getDevKey(u1_t *buf)
{
    memcpy_P(buf, APPKEY, 16);
}

unsigned long lastLocationTXMillis = 0;

static osjob_t sendjob;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

bool isJobRunning()
{
    return LMIC.opmode & OP_TXRXPEND;
}

bool isAcknowledgement()
{
    return LMIC.txrxFlags & TXRX_ACK;
}

void lora_send(osjob_t *j)
{
    if (isJobRunning())
    {
        LOG_MSG("Job is running...\n");

        return;
    }

    uint8_t data[LOCATION_LENGTH];
    int dataLength = 0;

    if ((!lastLocationTXMillis ||
        ((millis() - lastLocationTXMillis) % ULONG_MAX) > (TX_INTERVAL_LOCATION * 1000)) &&
        packLocationMessage(data)) {
        lastLocationTXMillis = millis() || (millis() + 1);
        dataLength = LOCATION_LENGTH;

        LOG_MSG("Location message packed\n");
    } else {
        dataLength = HEARTBEAT_LENGTH;

        packHeartbeatMessage(data);

        LOG_MSG("Heartbeat message packed\n");
    }

#ifdef SC_DEBUG
    Serial.print(F("Payload Uplink: "));
    for (unsigned int i = 0; i < sizeof(data); i++)
    {
        Serial.print(data[i], HEX);
        Serial.print(F(" "));
    };
    LOG_MSG("\n");
#endif

    automationEnabled = false;

    int result = LMIC_setTxData2(LORA_PORT, data, dataLength, LORA_NO_CONFIRMATION);

    if (result == 0)
    {
        LOG_MSG("Packet queued\n");
    }
    else
    {
        LOG_MSG("Packet not queued\n");
    }

}

void onIncomingData(int length, uint8_t *data)
{
  uint8_t protocolVersion = data[0] >> 4;

  if (protocolVersion != 0x0)
  {
      LOG_MSG("Unsupported protocol version\n");
      return;
  }

  uint8_t messageType = data[0] & 0xff;

  LOG_MSG("VERSION=%d TYPE=%d", protocolVersion, messageType);

  switch (messageType)
  {
    case CONFIGURATION_TYPE:
      if (length != 6)
      {
          LOG_MSG("Message length does not fit configuration message\n");
          return;
      }

      LOG_MSG("Incoming configuration message\n");

      processConfigurationMessage(&data[1]);

      checkAutomation(&global_config);
      break;
    default:
      LOG_MSG("Invalid message type received\n");
  }
}

void onTXComplete()
{
    unsigned int delayInSeconds = TX_INTERVAL;

    if (isAcknowledgement())
    {
        LOG_MSG("Received ack\n");
    }

    if (LMIC.dataLen)
    {
        onIncomingData(LMIC.dataLen, LMIC.frame + LMIC.dataBeg);

        delayInSeconds = 6;
    }

    // Schedule next transmission
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(delayInSeconds), lora_send);
}

void onEvent(ev_t ev)
{
    switch (ev)
    {
    case EV_TXSTART:
        LOG_MSG("EV_TXSTART\n");

        turnLedOn(STATUS_LED_TX);

        automationEnabled = false;
        break;
    case EV_TXCOMPLETE:
        LOG_MSG("EV_TXCOMPLETE (includes waiting for RX windows)\n");

        turnLedOff(STATUS_LED_TX);

        onTXComplete();

        automationEnabled = true;
        break;
    case EV_JOINED:
        LOG_MSG("EV_JOINED\n");

        turnLedOn(STATUS_LED_JOINED);

        LMIC_setAdrMode(0);
        LMIC_setLinkCheckMode(0);

        lora_send(&sendjob);
        break;
    case EV_JOINING:
        LOG_MSG("EV_JOINING\n");
        break;
    case EV_JOIN_TXCOMPLETE:
        LOG_MSG("EV_JOIN_TXCOMPLETE\n");

        turnLedOff(STATUS_LED_TX);

        break;
    case EV_REJOIN_FAILED:
        LOG_MSG("EV_REJOIN_FAILED\n");

        turnLedOff(STATUS_LED_JOINED);

        break;
#ifdef SC_DEBUG
    case EV_SCAN_TIMEOUT:
        LOG_MSG("EV_SCAN_TIMEOUT\n");
        break;
    case EV_BEACON_FOUND:
        LOG_MSG("EV_BEACON_FOUND\n");
        break;
    case EV_BEACON_MISSED:
        LOG_MSG("EV_BEACON_MISSED\n");
        break;
    case EV_BEACON_TRACKED:
        LOG_MSG("EV_BEACON_TRACKED\n");
        break;
    case EV_RFU1:
        LOG_MSG("EV_RFU1\n");
        break;
    case EV_JOIN_FAILED:
        LOG_MSG("EV_JOIN_FAILED\n");
        break;
    case EV_LOST_TSYNC:
        LOG_MSG("EV_LOST_TSYNC\n");
        break;
    case EV_RESET:
        LOG_MSG("EV_RESET\n");
        break;
    case EV_RXCOMPLETE:
        // data received in ping slot
        LOG_MSG("EV_RXCOMPLETE\n");
        break;
    case EV_LINK_DEAD:
        LOG_MSG("EV_LINK_DEAD\n");
        break;
    case EV_LINK_ALIVE:
        LOG_MSG("EV_LINK_ALIVE\n");
        break;
    default:
        LOG_MSG("Unknown event: %d\n", (unsigned) ev);
#endif
    }
}

void lora_send_immediately()
{
    LOG_MSG("Send immediately\n");

    os_clearCallback(&sendjob);

    os_setCallback(&sendjob, lora_send);
}

void lora_start()
{
    os_init();
    LMIC_reset();

    LMIC_setClockError(MAX_CLOCK_ERROR * 0.4 / 100);

    if (!LMIC_startJoining())
    {
        lora_send(&sendjob);
    }
}

void lora_once()
{
    os_runloop_once();
}