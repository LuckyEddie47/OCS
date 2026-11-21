// Bench test overrides, appended to a copy of your Config.h by build_sim.sh (never edit your real Config.h for this)
// Turns on the simulator and swaps the hardware the bench board doesn't have for simulated values.

#define SIMULATE                        ON

// no network for NTP, use the Teensy's own clock (the uploader sets it from the PC)
#undef  TIME_LOCATION_SOURCE
#define TIME_LOCATION_SOURCE        TEENSY

// the simulator supplies these readings, so the real sensor drivers are turned off
#undef  WEATHER_SENSOR_TP_BMP280
#define WEATHER_SENSOR_TP_BMP280       OFF
#undef  WEATHER_SENSOR_TPH_BME280
#define WEATHER_SENSOR_TPH_BME280      OFF
#undef  WEATHER_SENSOR_TH_SI7021
#define WEATHER_SENSOR_TH_SI7021       OFF
#undef  WEATHER_SENSOR_TH_DHT
#define WEATHER_SENSOR_TH_DHT          OFF
#undef  WEATHER_SENSOR_T_ANALOG
#define WEATHER_SENSOR_T_ANALOG        OFF
#undef  WEATHER_SENSOR_WIND_CUP
#define WEATHER_SENSOR_WIND_CUP        OFF
#undef  WEATHER_SENSOR_RAIN_GENERIC
#define WEATHER_SENSOR_RAIN_GENERIC    OFF
#undef  WEATHER_SENSOR_CLOUD_MLX90614
#define WEATHER_SENSOR_CLOUD_MLX90614  OFF
#undef  THERMOSTAT_SENSOR_TPH_BME280
#define THERMOSTAT_SENSOR_TPH_BME280   OFF
#undef  THERMOSTAT_SENSOR_TP_BMP280
#define THERMOSTAT_SENSOR_TP_BMP280    OFF
#undef  THERMOSTAT_SENSOR_TH_SI7021
#define THERMOSTAT_SENSOR_TH_SI7021    OFF
#undef  THERMOSTAT_SENSOR_TH_DHT
#define THERMOSTAT_SENSOR_TH_DHT       OFF
#undef  THERMOSTAT_SENSOR_T_ANALOG
#define THERMOSTAT_SENSOR_T_ANALOG     OFF

// things worth exercising on the bench
// (memory on device 3: in a config where a power device shares its relay with HEAT_RELAY, COOL_RELAY or
// HUMIDITY_RELAY the thermostat switches it at start up and every 5 minutes, so memory looks broken there)
#undef  POWER_DEVICE3_MEMORY
#define POWER_DEVICE3_MEMORY            ON
#undef  ROOF_AUTOCLOSE_DAWN
#define ROOF_AUTOCLOSE_DAWN             ON

// serial debug output goes to the first USB port, mixed in with command replies, so keep it quiet
#undef  DEBUG
#define DEBUG                          OFF
