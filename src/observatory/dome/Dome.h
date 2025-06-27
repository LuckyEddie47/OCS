// Dome control
#pragma once

#include "../../Common.h"

#ifdef DOME_PRESENT

#include "../../locales/Locale.h" 
#include "../../lib/axis/Axis.h"

enum ParkState: uint8_t {PS_UNPARKED, PS_PARKING, PS_PARKED, PS_PARK_FAILED, PS_UNPARKING};

#pragma pack(1)
typedef struct ParkCoordinate {
  ParkState state;
  float azimuth;               // in degrees
  float altitude;              // in degrees
} ParkCoordinate;

typedef struct BacklashCoordinate {
  float azimuth;               // in degrees
  float altitude;              // in degrees
} BacklashCoordinate;

#define DomeSettingsSize 17
typedef struct DomeSettings {
  ParkCoordinate park;
  BacklashCoordinate backlash;
} DomeSettings;
#pragma pack()

#ifdef AXIS1_DRIVER_PRESENT
  extern StepDirMotor motor1;
#elif defined(AXIS1_SERVO_PRESENT)
  extern ServoMotor motor1;
#endif
extern Axis axis1;

#if AXIS2_DRIVER_MODEL != OFF
  #ifdef AXIS2_DRIVER_PRESENT
    extern StepDirMotor motor2;
  #elif defined(AXIS2_SERVO_PRESENT)
    extern ServoMotor motor2;
  #endif
  extern Axis axis2;
#endif

class Dome {
  public:
    void init();

    // process commands associated with the ROOF
    bool command(char reply[], char command[], char parameter[], bool *supressFrame, bool *numericReply, CommandError *commandError);

    // reset dome at the home position
    void reset();

    // get dome azimuth (0 to 360 degrees)
    inline float getTargetAzimuth() { 
      float z = targetAzm;
      if (z < 0.0F) z += 360.0F;
      return z;
    }

    #if AXIS2_DRIVER_MODEL != OFF
      // get dome altitude (0 to 90 degrees)
      inline float getTargetAltitude() { return targetAlt; }

      // set dome goto target altitude (0 to 90 degrees)
      inline void setTargetAltitude(float alt) {
        targetAlt = axis2.getInstrumentCoordinate();
        if (alt >= (float)(AXIS2_LIMIT_MIN) && alt <= (float)(AXIS2_LIMIT_MAX)) targetAlt = alt;

        if (targetAlt < (float)(AXIS2_LIMIT_MIN)) targetAlt = (float)(AXIS2_LIMIT_MIN);
        if (targetAlt > (float)(AXIS2_LIMIT_MAX)) targetAlt = (float)(AXIS2_LIMIT_MAX);
      }
    #endif

    inline float dist(float z1, float z2) { return abs(z1 - z2); }

    // set dome goto target azimuth (0 to 360 degrees)
    inline void setTargetAzimuth(float azm) {
      float z1 = azm - 360.0F; // -360 to 0
      bool valid1 = (z1 >= (float)(AXIS1_LIMIT_MIN) && z1 <= (float)(AXIS1_LIMIT_MAX));

      float z2 = azm;          // 0 to 360
      bool valid2 = (z2 >= (float)(AXIS1_LIMIT_MIN) && z2 <= (float)(AXIS1_LIMIT_MAX));

      targetAzm = axis1.getInstrumentCoordinate();

      if (dist(targetAzm, z2) <= dist(targetAzm, z1) && valid2) {
        targetAzm = z2;
      } else
      if (dist(targetAzm, z1) <= dist(targetAzm, z2) && valid1) {
        targetAzm = z1;
      } else
      if (valid2) {
        targetAzm = z2;
      } else
      if (valid1) {
        targetAzm = z1;
      }

      if (targetAzm < (float)(AXIS1_LIMIT_MIN)) targetAzm = (float)(AXIS1_LIMIT_MIN);
      if (targetAzm > (float)(AXIS1_LIMIT_MAX)) targetAzm = (float)(AXIS1_LIMIT_MAX);
    }

    // get dome azimuth (0 to 360 degrees)
    inline float getAzimuth() {
      float z = axis1.getInstrumentCoordinate();
      if (z < 0.0F) z += 360.0F;
      return z;
    }

    // dome altitude (0 to 90 degrees, or NAN if inactive)
    inline float getAltitude() {
      #if AXIS2_DRIVER_MODEL != OFF
        return axis2.getInstrumentCoordinate();
      #else
       return NAN;
      #endif
    }

    // dome goto azimuth
    CommandError gotoAzimuthTarget();
    // dome sync azimuth
    CommandError syncAzimuthTarget();
    // dome goto altitude
    CommandError gotoAltitudeTarget();

    // move to the home position
    CommandError findHome();

    // stop slew
    void stop();

    // dome park
    CommandError park();
    // dome unpark
    CommandError unpark();
    // dome set park
    CommandError setpark();

    // check if dome is slewing
    bool isSlewing();
    // check if dome is parked
    bool isParked();

    // return dome status message
    const char* statusMessage();

    // poll dome to monitor motion
    void monitor();

    bool ready = false;

  private:
    DomeSettings settings = {
      {PS_UNPARKED, 0.0F, 0.0F},
      {0.0F, 0.0F}
    };

    float targetAzm = AXIS1_HOME_DEFAULT;
    float targetAlt = AXIS2_HOME_DEFAULT;

    bool homing = false;

    char statusMsg[40] = L_STOPPED;
};

extern Dome dome;

#endif
