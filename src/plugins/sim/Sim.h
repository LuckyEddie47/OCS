// Bench test simulator plugin, see README.md
#pragma once

#include "../../lib/commands/CommandErrors.h"

class Sim {
public:
  void init();

  bool command(char *reply, char *command, char *parameter, bool *supressFrame, bool *numericReply, CommandError *commandError);

  void poll();

private:
  void simulateRoof(float dt);
  void simulateMount(float dt);
  void simulateClimate(float dt);
  void writeSensors();

  // roof position, 0.0 is closed and 1.0 is open
  float roofPosition = 0.0F;
  float roofTravelSeconds = 15.0F;
  int roofDirection = 0;
  bool lastOpenRelay = false;
  bool lastCloseRelay = false;

  // mount, parked allows the roof to move (the roof interlock sense)
  bool mountParked = true;
  float parkCountdown = -1.0F;

  bool mainsOk = true;

  // inside (thermostat sensor)
  float insideTemperature = 12.0F;
  float insideHumidity = 65.0F;
  float insidePressure = 1010.0F;

  // outside (weather sensors)
  float outsideTemperature = 8.0F;
  float skyTemperature = -20.0F;
  float windSpeed = 6.0F;
  float outsidePressure = 1012.0F;
  float outsideHumidity = 80.0F;
  float rainState = 3.0F; // 1 wet, 2 warn, 3 dry
  float skyQuality = 20.5F;

  unsigned long lastPollMs = 0;
};

extern Sim sim;
