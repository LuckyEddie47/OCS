// -----------------------------------------------------------------------------------------------------------------
// gets rain sensor info. 1 is Rain, 2 is Warn, and 3 is Dry
// this is for the typical eBay rain sensor like this one:
// http://www.ebay.com/itm/Raindrops-Detection-sensor-modue-rain-module-weather-module-Humidity-For-Arduino-/400439668701

#include "RainGeneric.h"

#if defined(WEATHER_SENSOR_RAIN_GENERIC) && WEATHER_SENSOR_RAIN_GENERIC != OFF

#include "../../lib/tasks/OnTask.h"
#include "../analog/Analog.h"

extern float _rainSense;
extern bool _rainSenseAssigned;
extern char _rainSenseName[40];

void rainGenericWrapper() { rainGeneric.poll(); }

// setup anemometer
bool RainGeneric::init() {
  if (active) return true;

  if (_rainSenseAssigned) return false;

  VF("MSG: RainGeneric, start monitor task (default rate priority 7)... ");
  if (tasks.add(WEATHER_SENSOR_SAMPLE_PERIOD, 0, true, 7, rainGenericWrapper,"weaRain")) {
    VLF("success");
    _rainSenseAssigned = true;
    strcpy(_rainSenseName, "Generic Rain Sensor on Analog Input");
    active = true;
  } else { VLF("FAILED!"); }

  return active;
}

// update
void RainGeneric::poll() {
  if (!active) return;

  float rain = analog.read(WEATHER_SENSOR_RAIN_GENERIC);

  if (rain <= WEATHER_SENSOR_RAIN_LOW) _rainSense = 1.0F; else
  if (rain > WEATHER_SENSOR_RAIN_HIGH) _rainSense = 3.0F; else _rainSense = 2.0F;
}

RainGeneric rainGeneric;

#endif
