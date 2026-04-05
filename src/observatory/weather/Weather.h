// -----------------------------------------------------------------------------------
// Weather/Safety monitor and logging
#pragma once

#include "../../Common.h"

#ifdef WEATHER_PRESENT

#include "../../libApp/commands/ProcessCmds.h"

// this is the response time required to cover approximately 2/3 of a change in cloud temperature
// adjust higher for less sensitivity to passing clouds/changing conditions, lower for more sensitivity
#define AvgTimeSeconds 600.0

class Weather {
  public:
    bool command(char reply[], char command[], char parameter[], bool *supressFrame, bool *numericReply, CommandError *commandError);

    void init();

    float getSkyDiffTemp();

    float getAvgSkyDiffTemp();

    // gets unfiltered wind speed at last poll in kph (NAN if unavailable)
    float getWindspeedInstant() { return windspeedInstant; }

    // gets peak wind speed since last log write in kph (NAN if unavailable)
    float getWindspeedPeak() { return windspeedPeak; }

    // gets cloud cover in %
    float cloudCover();

    // get cloud cover text
    const char * cloudCoverDescription();

    bool isSafe();

    void poll();

  private:
    int getAvgSkyDiffTempIndex();
    void dtostrf2(float d, int i, int i1, float l, float h, char result[]);

    long TimeSeconds = 0;
    unsigned long last = 10000;

    float skyDiffTemp = NAN;
    float avgSkyDiffTemp = NAN;
    float sa = NAN;
    float ss = NAN;
    float sad = NAN;
    float lad = NAN;
    float wa = NAN;
    float windspeedInstant = NAN; // unfiltered reading at last poll (for future use)
    float windspeedPeak = NAN;    // max raw reading since last log write

    const char *CloudDescription[10] =
    { "Invalid", "Clear", "Mostly-Clr", "Slight-Hze", "Ovrcst/Hze",
      "Overcast", "Cld/Ovrcst", "Mstly-Cldy", "Cloudy", "Very-Cldy" };
};

extern Weather weather;

#endif
