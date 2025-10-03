// -----------------------------------------------------------------------------------------------------------------
// Observatory thermostat
#pragma once

#include "../../Common.h"

#ifdef THERMOSTAT_PRESENT

#include "../../libApp/commands/ProcessCmds.h"

class Thermostat {
  public:
    bool command(char reply[], char command[], char parameter[], bool *supressFrame, bool *numericReply, CommandError *commandError);

    void init();
    void poll();

    void setHeatSetpoint(float value);
    float getHeatSetpoint();

    void setCoolSetpoint(float value);
    float getCoolSetpoint();

    float getTemperature();
    float getHumidity();

    void setHumiditySetpoint(float value);
    float getHumiditySetpoint();

  private:
    float averageTemperature = NAN;
    float t1 = NAN;
    float t2 = NAN;
    float humidity = NAN;
};

extern Thermostat thermostat;

#endif
