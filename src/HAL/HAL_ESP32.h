// Platform setup ------------------------------------------------------------------------------------
#pragma once

// This is for fast processors with hardware FP
#define HAL_FAST_PROCESSOR

// 1/200 second sidereal timer
#define HAL_FRACTIONAL_SEC 200.0F

// volatage for ADC conversions
#define HAL_VCC 3.3F

// Analog read and write
#ifndef ANALOG_READ_RANGE
  #define ANALOG_READ_RANGE 4095
#endif
#ifndef ANALOG_WRITE_RANGE
  #define ANALOG_WRITE_RANGE 1023
#endif
#ifndef ANALOG_WRITE_PWM_BITS
  #define ANALOG_WRITE_PWM_BITS 10
#endif

// Lower limit (fastest) step rate in uS for this platform (in SQW mode) and width of step pulse
#define HAL_MAXRATE_LOWER_LIMIT 16
#define HAL_PULSE_WIDTH 200  // in ns, measured 1/18/22

// New symbol for the default I2C port -------------------------------------------------------------
#include <Wire.h>
#define HAL_Wire Wire
#ifndef HAL_WIRE_CLOCK
  #define HAL_WIRE_CLOCK 20000
#endif

// Non-volatile storage ----------------------------------------------------------------------------
#if NV_DRIVER == NV_DEFAULT
  #define E2END 4095
  #define NV_ENDURANCE NVE_LOW
  #include "../lib/nv/NV_ESP.h"
  #define HAL_NV_INIT() nv.init(E2END + 1, false, 5000, false);
#endif

//--------------------------------------------------------------------------------------------------
// General purpose initialize for HAL, optionally also early init of SERIAL_SIP/PIP or SERIAL_BT

#if SERIAL_BT_MODE == SLAVE
  #include <BluetoothSerial.h>
  extern BluetoothSerial bluetoothSerial;
  #define SERIAL_BT bluetoothSerial
#endif
#if defined(SERIAL_BT)
  #define SERIAL_BT_BEGIN() SERIAL_BT.begin(SERIAL_BT_NAME);
#else
  #define SERIAL_BT_BEGIN()
#endif

#if SERIAL_BT_MODE != OFF && SERIAL_IP_MODE != OFF
  #error "Configuration (Config.h): SERIAL_BT_MODE and SERIAL_IP_MODE can't be enabled at the same time, disable one or both options."
#endif

#define HAL_INIT() { \
  analogWriteResolution(ANALOG_WRITE_PWM_BITS); \
  SERIAL_BT_BEGIN(); \
}

//--------------------------------------------------------------------------------------------------
// Internal MCU temperature (in degrees C)
// Correction for ESP32's internal temperture sensor
#define INTERNAL_TEMP_CORRECTION 0
#define HAL_TEMP() ( temperatureRead() + INTERNAL_TEMP_CORRECTION )

//---------------------------------------------------------------------------------------------------
// Misc. includes to support this processor's operation
#include "../lib/watchdog/Watchdog.h"
#include "../lib/analog/AN_ESP32.h"

// Allow MCU reset -----------------------------------------------------------------------------------
#define HAL_RESET() ESP.restart()

// Watchdog control macros --------------------------------------------------------------------------
#if WATCHDOG != OFF
  #define WDT_ENABLE watchdog.enable(8);
  #define WDT_RESET watchdog.reset();
  #define WDT_DISABLE watchdog.disable();
#else
  #define WDT_ENABLE
  #define WDT_RESET
  #define WDT_DISABLE
#endif

// stand-in for delayNanoseconds()
#define delayNanoseconds(ns) delayMicroseconds(ceilf(ns/1000.0F))
