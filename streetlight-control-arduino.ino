#include <TinyGPS.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define LORA_PORT 1
#define LORA_REQ_CONFIRMATION 1
#define LORA_NO_CONFIRMATION 0
#define PROTOCOL_VERSION 0
#define HEARTBEAT_TYPE 1
#define CONFIGURATION_TYPE 2
#define DEFAULT_LATITUDE 47.63249
#define DEFAULT_LONGITUDE 9.52935
#define BAUD_RATE 9600
#define LOG(message) Serial.println(F(message))

#define RELAY0_PIN A0
#define RELAY1_PIN A1
#define RELAY2_PIN A2
#define RELAY3_PIN A3

#define RELAY_MODE_OFF 0
#define RELAY_MODE_ON 1
#define RELAY_MODE_AUTO 2

#define MAX_TIME_BYTE 239

TinyGPS gps;
SoftwareSerial ss(4, 3); // Arduino RX, TX to conenct GPS

typedef struct {
  int year;
  byte month;
  byte day;
  byte hour;
  byte minute;
  byte second;
} s_date;

typedef struct {
  byte hour;
  byte minute;
} s_time;

typedef struct {
  float longitude;
  float latitude;
} s_coords;

unsigned long startMillis;



// =================== LORA =======================

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = {0xB0, 0xD3, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70};
void os_getArtEui(u1_t *buf) {
  memcpy_P(buf, APPEUI, 8);
}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = {0xE8, 0x94, 0x66, 0xCD, 0x81, 0x7F, 0x52, 0x00};
void os_getDevEui(u1_t *buf) {
  memcpy_P(buf, DEVEUI, 8);
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = {0x26, 0x49, 0x42, 0xBF, 0x56, 0xFB, 0x9C, 0x6E, 0x12, 0xF1, 0xE3, 0xA1, 0x7F, 0x86, 0xA3, 0x2C};
void os_getDevKey(u1_t *buf) {
  memcpy_P(buf, APPKEY, 16);
}

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 120;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 9,
  .dio = {2, 6, 7},
};

static uint8_t heartbeatData[7];
uint8_t relayModes = 0;
uint8_t timeOn = 0;
uint8_t timeOff = 0;
uint8_t outageOn = 0;
uint8_t outageOff = 0;

const int RELAY_PINS[] = {RELAY0_PIN, RELAY1_PIN, RELAY2_PIN, RELAY3_PIN};

bool packHeartbeatData()
{
  s_coords coords;
  s_date date;
  bool hasGPSSignal = GPSRead(&coords, &date);

  int maintenanceMode = 0;
  int gpsSignal = hasGPSSignal ? 1 << 6 : 0;
  int relayStates = digitalRead(RELAY0_PIN) + (digitalRead(RELAY1_PIN) << 1) + (digitalRead(RELAY2_PIN) << 2) + (digitalRead(RELAY3_PIN) << 3);

  heartbeatData[0] = (PROTOCOL_VERSION << 4) + HEARTBEAT_TYPE;
  heartbeatData[1] = 0; // checksum
  heartbeatData[2] = 0; // hour
  heartbeatData[3] = 0; // minute
  heartbeatData[4] = (TX_INTERVAL / 60);
  heartbeatData[5] = maintenanceMode + gpsSignal + relayStates;
  heartbeatData[6] = 0;
}

bool isJobRunning() {
  return LMIC.opmode & OP_TXRXPEND;
}

void do_send(osjob_t *j)
{
  if (isJobRunning())
  {
    return;
  }

  checkAutomation();

  packHeartbeatData();

#ifdef DEBUG
  Serial.print(F("Payload Uplink: "));
  for (int i = 0; i < sizeof(heartbeatData); i++)
  {
    Serial.print(heartbeatData[i], HEX);
    Serial.print(F(" "));
  };
  LOG("");
#endif

  LMIC_setTxData2(LORA_PORT, heartbeatData, sizeof(heartbeatData), LORA_NO_CONFIRMATION);

  LOG("Packet queued");
}

uint8_t getRelayMode(uint8_t modes, short number)
{
  return (modes >> (2 * number)) & 0x3;
}

void switchRelayOff(short number)
{
  LOG("Switch relay OFF");
  digitalWrite(RELAY_PINS[number], LOW);
}

void switchRelayOn(short number)
{
  LOG("Switch relay ON");
  digitalWrite(RELAY_PINS[number], HIGH);
}

void onConfigurationMessage(uint8_t *data)
{
  LOG("Configuration message received");

  relayModes = data[0];

  timeOn = data[1];
  timeOff = data[2];
  outageOn = data[3];
  outageOff = data[4];

  for (short i = 0; i < 4; i++)
  {
    uint8_t mode = getRelayMode(relayModes, i);

    if (mode == RELAY_MODE_OFF)
    {
      switchRelayOff(i);
    } else if (mode == RELAY_MODE_ON)
    {
      switchRelayOn(i);
    }
  }
}

void onIncomingData(int length, uint8_t *data)
{
  Serial.print(F("Received "));
  Serial.print(length);
  Serial.println(F(" bytes of payload"));

  Serial.print(F("Payload downlink 2: "));
  for (int i = 0; i < length; i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(F(" "));
  };
  LOG("");

  uint8_t protocolVersion = data[0] >> 4;
  uint8_t messageType = data[0] & 0xff;

  //TODO check message length

  Serial.print(F("VERSION="));
  Serial.print(protocolVersion);
  Serial.print(F(" TYPE="));
  Serial.println(messageType);

  switch (messageType)
  {
    case CONFIGURATION_TYPE:
      LOG("Incoming configuration message");

      onConfigurationMessage(&data[1]);
      break;
    default:
      LOG("Invalid message type received");
  }
}

uint8_t timeToByte(s_date input)
{
  return ((input.hour % 24) * 10) + (int (input.minute / 6));
}

bool isInRange(uint8_t start, uint8_t ende, uint8_t value)
{
  return (value > start && value <= MAX_TIME_BYTE && ende < start) ||
         (value < ende && value >= 0 && ende < start) ||
         (value > start && value < ende);
}

void checkAutomation() {
  s_date date;
  bool isValid = GPSReadDateTime(&date);

  if (!isValid)
  {
    return;
  }

  uint8_t timeByte = timeToByte(date);
  bool isInOnRange = false;

  if (timeOn != timeOff)
  {
    if (outageOn != outageOff)
    {
      isInOnRange = isInRange(timeOn, outageOn, timeByte) || isInRange(outageOff, timeOff, timeByte);
    }
    else
    {
      isInOnRange = isInRange(timeOn, timeOff, timeByte);
    }
  }

  for (short i = 0; i < 4; i++)
  {
    if (getRelayMode(relayModes, i) != RELAY_MODE_AUTO)
    {
      continue;
    }

    if (isInOnRange)
    {
      switchRelayOn(i);
    }
    else
    {
      switchRelayOff(i);
    }
  }
}

void onEvent(ev_t ev)
{
  Serial.print(F("##### "));
  Serial.println(digitalRead(A0));

  Serial.print(os_getTime());
  Serial.print(": ");

  switch (ev)
  {
    case EV_TXCOMPLETE:
      LOG("EV_TXCOMPLETE (includes waiting for RX windows)");

      if (LMIC.txrxFlags & TXRX_ACK)
      {
        LOG("Received ack");
      }

      if (LMIC.dataLen)
      {
        onIncomingData(LMIC.dataLen, LMIC.frame + LMIC.dataBeg);
        os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(5), do_send);
        break;
      }

      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_SCAN_TIMEOUT:
      LOG("EV_SCAN_TIMEOUT");
      break;
    case EV_BEACON_FOUND:
      LOG("EV_BEACON_FOUND");
      break;
    case EV_BEACON_MISSED:
      LOG("EV_BEACON_MISSED");
      break;
    case EV_BEACON_TRACKED:
      LOG("EV_BEACON_TRACKED");
      break;
    case EV_JOINING:
      LOG("EV_JOINING");
      break;
    case EV_JOINED:
      LOG("EV_JOINED");

      // Disable link check validation (automatically enabled
        // during join, but not supported by TTN at this time).
        LMIC_setLinkCheckMode(0);
      break;
    case EV_RFU1:
      LOG("EV_RFU1");
      break;
    case EV_JOIN_FAILED:
      LOG("EV_JOIN_FAILED");
      break;
    case EV_REJOIN_FAILED:
      LOG("EV_REJOIN_FAILED");
      break;
    case EV_LOST_TSYNC:
      LOG("EV_LOST_TSYNC");
      break;
    case EV_RESET:
      LOG("EV_RESET");
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      LOG("EV_RXCOMPLETE");
      break;
    case EV_LINK_DEAD:
      LOG("EV_LINK_DEAD");
      break;
    case EV_LINK_ALIVE:
      LOG("EV_LINK_ALIVE");
      break;
    default:
      LOG("Unknown event");
  }
}






void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
    {
      gps.encode(ss.read());
    }
  } while (millis() - start < ms);
}

bool GPSReadDateTime(s_date *date)
{
  unsigned long age;
  byte hundredths;
  gps.crack_datetime(date->year, date->month, date->day, date->hour, date->minute, date->second, &hundredths, &age);

  return age != TinyGPS::GPS_INVALID_AGE;
}

bool GPSRead(s_coords *coords, s_date *date)
{
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);

#ifdef DEBUG
  Serial.print("INVALID=");
  Serial.print(TinyGPS::GPS_INVALID_F_ANGLE);
  Serial.print(" LAT=");
  Serial.print(flat);
  Serial.print(" LON=");
  Serial.print(flon);
  Serial.print(" SAT=");
  Serial.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
#endif

  if (flat >= TinyGPS::GPS_INVALID_F_ANGLE || flon >= TinyGPS::GPS_INVALID_F_ANGLE)
  {
    return false;
  }

  coords->latitude = flat;
  coords->longitude =  flon;

  return GPSReadDateTime(date);
}

void packGPSCoordinates(float latitude, float longitude, uint8_t *data)
{
  int32_t lat = (latitude , 6) * 10000; //save six decimal places
  int32_t lon = (longitude, 6) * 10000;

  data[0] = lat >> 16;
  data[1] = lat >> 8;
  data[2] = lat;
  data[3] = lon >> 16;
  data[4] = lon >> 8;
  data[5] = lon;
}

void setup() {
  Serial.begin(BAUD_RATE);

  LOG("Starting... [otaa-gps-device]");

  pinMode(RELAY0_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);

  Serial.println(digitalRead(RELAY0_PIN));

  digitalWrite(RELAY0_PIN, HIGH);

  Serial.println(digitalRead(RELAY0_PIN));

  startMillis = millis();

  os_init();
  LMIC_reset();

  do_send(&sendjob);
}

void loop() {
  os_runloop_once();

  //Serial.print(F("##### "));
  //Serial.println(digitalRead(A0));
}
