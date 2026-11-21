// Bench test simulator plugin, see README.md
//
// Drives the sense inputs and sensor readings so OCS can be run on a bare board:
//   - roof limit switches follow the roof motor relays
//   - the roof interlock (mount parked) follows the mount park relay
//   - inside temperature and humidity respond to the heat, cool and dehumidifier relays
//   - outside weather values are fixed until changed with commands
// Everything here can be overridden with the :Z...# commands below.

#include "Sim.h"
#include "../../Common.h"

#if SIMULATE == ON

#include "../../lib/tasks/OnTask.h"
#include "../../lib/sense/Sense.h"
#include "../../libApp/relay/Relay.h"

#if TIME_LOCATION_SOURCE == TEENSY
  #include <TimeLib.h>
#endif

// sensor readings, normally written by the sensor drivers
extern float _inside_temperature, _inside_humidity, _inside_pressure;
extern float _temperature, _skyTemperature, _windspeed, _pressure, _humidity, _rainSense, _skyQuality;
extern char _temperatureName[], _skyTemperatureName[], _windSpeedName[], _pressureName[];
extern char _humidityName[], _rainSenseName[], _skyQualityName[];

// sense inputs forced with :ZSn,0# or :ZSn,1#, -1 when the simulator (or the real pin) decides
static int8_t forcedSense[SENSE_MAX + 1];

static void setSense(int n, bool state) {
  if (n < 1 || n > SENSE_MAX) return;
  sense.simulate(n, forcedSense[n] >= 0 ? forcedSense[n] : (state ? 1 : 0));
}

void simWrapper() { sim.poll(); }

void Sim::init() {
  VLF("MSG: Plugins, starting: sim (SIMULATE is ON, sensors and switches are simulated)");

  for (int i = 0; i <= SENSE_MAX; i++) forcedSense[i] = -1;

  strcpy(_temperatureName, "Simulated");
  strcpy(_skyTemperatureName, "Simulated");
  strcpy(_windSpeedName, "Simulated");
  strcpy(_pressureName, "Simulated");
  strcpy(_humidityName, "Simulated");
  strcpy(_rainSenseName, "Simulated");
  strcpy(_skyQualityName, "Simulated");

  // the TEENSY time source sets the RTC but never reads it back, so pick up the time after a restart here
  #if TIME_LOCATION_SOURCE == TEENSY
    if (Teensy3Clock.get() > 1640995200UL) setTime(Teensy3Clock.get()); // only if after 2022-01-01
  #endif

  lastPollMs = millis();
  poll();

  tasks.add(100, 0, true, 6, simWrapper, "Sim");
}

void Sim::poll() {
  unsigned long nowMs = millis();
  float dt = (nowMs - lastPollMs)/1000.0F;
  lastPollMs = nowMs;

  simulateRoof(dt);
  simulateMount(dt);
  simulateClimate(dt);

  #if STAT_MAINS_SENSE != OFF
    setSense(STAT_MAINS_SENSE, mainsOk);
  #endif

  writeSensors();
}

void Sim::simulateRoof(float dt) {
  #ifdef ROOF_PRESENT
    bool openRelay = false;
    bool closeRelay = false;
    #if ROOF_MOTOR_OPEN_RELAY != OFF
      openRelay = relay.isOn(ROOF_MOTOR_OPEN_RELAY);
    #endif
    #if ROOF_MOTOR_CLOSE_RELAY != OFF
      closeRelay = relay.isOn(ROOF_MOTOR_CLOSE_RELAY);
    #endif

    #if ROOF_MOTOR_RELAY_MOMENTARY == ON
      // opener with push buttons: each press starts the roof moving, or stops it if it's moving
      if (openRelay && !lastOpenRelay) {
        if (roofDirection != 0) roofDirection = 0; else
        if (ROOF_SINGLE_OPEN_CLOSE_RELAY == ON) roofDirection = roofPosition >= 0.5F ? -1 : 1; else roofDirection = 1;
      }
      if (closeRelay && !lastCloseRelay) {
        if (roofDirection != 0) roofDirection = 0; else roofDirection = -1;
      }
    #else
      // motor relays: the roof moves while a relay is on
      if (openRelay && !closeRelay) roofDirection = 1; else
      if (closeRelay && !openRelay) roofDirection = -1; else roofDirection = 0;
    #endif
    lastOpenRelay = openRelay;
    lastCloseRelay = closeRelay;

    roofPosition += roofDirection*dt/roofTravelSeconds;
    if (roofPosition >= 1.0F) { roofPosition = 1.0F; if (ROOF_MOTOR_RELAY_MOMENTARY == ON) roofDirection = 0; }
    if (roofPosition <= 0.0F) { roofPosition = 0.0F; if (ROOF_MOTOR_RELAY_MOMENTARY == ON) roofDirection = 0; }

    #if ROOF_LIMIT_OPENED_SENSE != OFF
      setSense(ROOF_LIMIT_OPENED_SENSE, roofPosition >= 1.0F);
    #endif
    #if ROOF_LIMIT_CLOSED_SENSE != OFF
      setSense(ROOF_LIMIT_CLOSED_SENSE, roofPosition <= 0.0F);
    #endif
  #else
    UNUSED(dt);
  #endif
}

void Sim::simulateMount(float dt) {
  #if defined(ROOF_PRESENT) && defined(ROOF_MOUNT_PARK_RELAY) && ROOF_MOUNT_PARK_RELAY != OFF
    // the park relay asks the mount to park, it gets there a few seconds later
    if (relay.isOn(ROOF_MOUNT_PARK_RELAY) && !mountParked && parkCountdown < 0.0F) parkCountdown = 5.0F;
    if (parkCountdown >= 0.0F) {
      parkCountdown -= dt;
      if (parkCountdown < 0.0F) mountParked = true;
    }
  #else
    UNUSED(dt);
  #endif

  #if defined(ROOF_PRESENT) && ROOF_INTERLOCK_SENSE != OFF
    setSense(ROOF_INTERLOCK_SENSE, mountParked);
  #endif
}

void Sim::simulateClimate(float dt) {
  // with nothing running the inside drifts toward the outside conditions
  float targetTemperature = outsideTemperature;
  float targetHumidity = outsideHumidity;
  float temperatureRate = 0.002F; // fraction of the difference per second
  float humidityRate = 0.002F;

  #if HEAT_RELAY != OFF
    if (relay.isOn(HEAT_RELAY)) { targetTemperature = 30.0F; temperatureRate = 0.01F; }
  #endif
  #if COOL_RELAY != OFF
    if (relay.isOn(COOL_RELAY)) { targetTemperature = outsideTemperature - 2.0F; temperatureRate = 0.01F; }
  #endif
  #if HUMIDITY_RELAY != OFF
    if (relay.isOn(HUMIDITY_RELAY)) { targetHumidity = 30.0F; humidityRate = 0.01F; }
  #endif

  insideTemperature += (targetTemperature - insideTemperature)*temperatureRate*dt;
  insideHumidity += (targetHumidity - insideHumidity)*humidityRate*dt;
}

void Sim::writeSensors() {
  _inside_temperature = insideTemperature;
  _inside_humidity = insideHumidity;
  _inside_pressure = insidePressure;

  _temperature = outsideTemperature;
  _skyTemperature = skyTemperature;
  // a little gustiness so the wind chart isn't a flat line
  _windspeed = windSpeed > 0.0F ? max(0.0F, windSpeed + random(-10, 11)/10.0F) : 0.0F;
  _pressure = outsidePressure;
  _humidity = outsideHumidity;
  _rainSense = rainState;
  _skyQuality = skyQuality;
}

// :ZSn,s#   force sense n (1..8) on (s=1) or off (s=0), or s=A to let the simulator/pin decide again
// :ZIx,v#   inside value:  x=T temperature (C), H humidity (%), P pressure (mb)
// :ZOx,v#   outside value: x=T temperature (C), S sky temperature (C), W wind (kph), P pressure (mb),
//                          H humidity (%), R rain (1 wet, 2 warn, 3 dry), Q sky quality (mag/arcsec^2)
// :ZMn#     mount parked (n=1) or unparked (n=0)
// :ZPn#     mains power ok (n=1) or failed (n=0)
// :ZRn#     roof travel time n seconds (2..600) end to end
// :ZGn#     set roof position n percent (0..100), e.g. to test a roof stopped part way
// :Z?#      status, returns roof position %, direction, mount parked, inside T and H
bool Sim::command(char *reply, char *command, char *parameter, bool *supressFrame, bool *numericReply, CommandError *commandError) {
  UNUSED(supressFrame);

  if (command[0] != 'Z') return false;

  switch (command[1]) {
    case 'S': {
      int n = atoi(parameter);
      char *comma = strchr(parameter, ',');
      if (n < 1 || n > SENSE_MAX || comma == NULL) { *commandError = CE_PARAM_FORM; return true; }
      char s = comma[1];
      if (s == 'A' || s == 'a') forcedSense[n] = -1; else
      if (s == '0') forcedSense[n] = 0; else
      if (s == '1') forcedSense[n] = 1; else { *commandError = CE_PARAM_FORM; return true; }
      // senses the simulator doesn't drive go back to reading their pin
      sense.simulate(n, forcedSense[n]);
      poll();
      return true;
    }

    case 'I':
    case 'O': {
      if (parameter[0] == 0 || parameter[1] != ',') { *commandError = CE_PARAM_FORM; return true; }
      float v = atof(&parameter[2]);
      if (command[1] == 'I') {
        switch (parameter[0]) {
          case 'T': insideTemperature = v; break;
          case 'H': insideHumidity = v; break;
          case 'P': insidePressure = v; break;
          default: *commandError = CE_PARAM_FORM; return true;
        }
      } else {
        switch (parameter[0]) {
          case 'T': outsideTemperature = v; break;
          case 'S': skyTemperature = v; break;
          case 'W': windSpeed = v; break;
          case 'P': outsidePressure = v; break;
          case 'H': outsideHumidity = v; break;
          case 'R': if (v != 1.0F && v != 2.0F && v != 3.0F) { *commandError = CE_PARAM_RANGE; return true; } rainState = v; break;
          case 'Q': skyQuality = v; break;
          default: *commandError = CE_PARAM_FORM; return true;
        }
      }
      writeSensors();
      return true;
    }

    case 'M':
      if (parameter[0] == '1') { mountParked = true; parkCountdown = -1.0F; } else
      if (parameter[0] == '0') { mountParked = false; parkCountdown = -1.0F; } else { *commandError = CE_PARAM_FORM; return true; }
      poll();
      return true;

    case 'P':
      if (parameter[0] == '1') mainsOk = true; else
      if (parameter[0] == '0') mainsOk = false; else { *commandError = CE_PARAM_FORM; return true; }
      poll();
      return true;

    case 'R': {
      float t = atof(parameter);
      if (t < 2.0F || t > 600.0F) { *commandError = CE_PARAM_RANGE; return true; }
      roofTravelSeconds = t;
      return true;
    }

    case 'G': {
      int p = atoi(parameter);
      if (parameter[0] == 0 || p < 0 || p > 100) { *commandError = CE_PARAM_RANGE; return true; }
      roofPosition = p/100.0F;
      poll();
      return true;
    }

    case '?':
      if (parameter[0] != 0) return false;
      sprintf(reply, "roof %d%% dir %d mount %s Ti %.1f Hi %.0f",
              (int)round(roofPosition*100.0F), roofDirection, mountParked ? "P" : "U", insideTemperature, insideHumidity);
      *numericReply = false;
      return true;
  }

  return false;
}

Sim sim;

#endif
