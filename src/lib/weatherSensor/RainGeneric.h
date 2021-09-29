// -----------------------------------------------------------------------------------------------------------------
// gets rain sensor info. 1 is Rain, 2 is Warn, and 3 is Dry
// this is for the typical eBay rain sensor like this one:
// http://www.ebay.com/itm/Raindrops-Detection-sensor-modue-rain-module-weather-module-Humidity-For-Arduino-/400439668701
#pragma once

#include "../../Common.h"

#if WEATHER_SENSOR_RAIN_GENERIC != OFF

#define WEATHER_SENSOR_RAIN_LOW 0.25
#define WEATHER_SENSOR_RAIN_HIGH 0.75

// setup anemometer
class RainGeneric {
  public:
    bool init();
    void poll();

  private:
    bool active = false;
};

extern RainGeneric rainGeneric;

#endif