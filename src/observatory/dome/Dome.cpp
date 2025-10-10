// Dome control
#include "Dome.h"

#ifdef DOME_PRESENT

#include "../../lib/tasks/OnTask.h"
#include "../../lib/nv/Nv.h"

#include "../../observatory/Observatory.h"
#include "../roof/Roof.h"

void domeWrapper() { dome.monitor(); }

// initialize dome
void Dome::init() {
  // wait a moment for any background processing that may be needed
  delay(1000);

  // confirm the data structure size
  if (DomeSettingsSize < sizeof(DomeSettings)) { nv.initError = true; DL("ERR: Dome::init(); DomeSettingsSize error"); }

  // get settings stored in NV ready
  if (!nv.hasValidKey()) {
    VLF("MSG: Dome, writing defaults to NV");
  }

  nv.readBytes(NV_DOME_SETTINGS_BASE, &settings, sizeof(DomeSettings));

  if (settings.backlash.azimuth  < 0.0F)  { settings.backlash.azimuth = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV backlash.azimuth < 0 deg (set to 0)"); }
  if (settings.backlash.azimuth  > 10.0F) { settings.backlash.azimuth = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV backlash.azimuth > 10 deg (set to 0)"); }
  if (settings.backlash.altitude < 0.0F)  { settings.backlash.altitude = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV backlash.altitude < 0 deg (set to 0)"); }
  if (settings.backlash.altitude > 10.0F) { settings.backlash.altitude = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV backlash.altitude > 10 deg steps (set to 0)"); }
  if (settings.park.azimuth  < AXIS1_LIMIT_MIN) { settings.park.azimuth = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV park pos < AXIS1_LIMIT_MIN (set to 0.0)"); }
  if (settings.park.azimuth  > AXIS1_LIMIT_MAX) { settings.park.azimuth = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV park pos > AXIS1_LIMIT_MAX (set to 0.0)"); }
  if (settings.park.altitude < AXIS2_LIMIT_MIN) { settings.park.altitude = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV park pos < AXIS2_LIMIT_MIN (set to 0.0)"); }
  if (settings.park.altitude > AXIS2_LIMIT_MAX) { settings.park.altitude = 0.0F; initError.value = true; DLF("ERR, Dome.init(): bad NV park pos > AXIS2_LIMIT_MAX (set to 0.0)"); }

  VLF("MSG: Dome, init (Axis1)");
  if (!axis1.init(&motor1)) { DLF("ERR: Axis1, no motion controller exiting!"); return; }
  axis1.resetPositionSteps(round(AXIS1_HOME_DEFAULT*AXIS1_STEPS_PER_DEGREE));
  axis1.setBacklash(settings.backlash.azimuth);
  axis1.setFrequencyMax(AXIS1_SLEW_RATE_DESIRED);
  axis1.setFrequencyMin(0.01F);
  axis1.setFrequencySlew(AXIS1_SLEW_RATE_DESIRED);
  axis1.setSlewAccelerationTime(AXIS1_ACCELERATION_TIME);
  axis1.setSlewAccelerationTimeAbort(AXIS1_RAPID_STOP_TIME);
  if (AXIS1_POWER_DOWN == ON) axis1.setPowerDownTime(DEFAULT_POWER_DOWN_TIME);

  if (AXIS1_WRAP == ON) {
    axis1.coordinateWrap(360.0);
    axis1.settings.limits.min = -360.0;
    axis1.settings.limits.max = 360.0;
  }

  #if AXIS2_DRIVER_MODEL != OFF
    VLF("MSG: Dome, init (Axis2)");
    if (!axis2.init(&motor2)) { DLF("ERR: Axis2, no motion controller exiting!"); return; }
    axis2.resetPositionSteps(round(AXIS2_HOME_DEFAULT*AXIS2_STEPS_PER_DEGREE));
    axis2.setBacklash(settings.backlash.altitude);
    axis2.setFrequencyMax(AXIS2_SLEW_RATE_DESIRED);
    axis2.setFrequencyMin(0.01F);
    axis2.setFrequencySlew(AXIS2_SLEW_RATE_DESIRED);
    axis2.setSlewAccelerationTime(AXIS2_ACCELERATION_TIME);
    axis2.setSlewAccelerationTimeAbort(AXIS2_RAPID_STOP_TIME);
    if (AXIS2_POWER_DOWN == ON) axis2.setPowerDownTime(DEFAULT_POWER_DOWN_TIME);
  #endif

  // handle parked state
  if (settings.park.state == PS_PARKED) {
    axis1.setInstrumentCoordinate(settings.park.azimuth);
    #if AXIS2_DRIVER_MODEL != OFF
      axis2.setInstrumentCoordinate(settings.park.altitude);
    #endif
  }

  // start monitor task
  VF("MSG: Dome, start monitor task (rate 1s priority 6)... ");
  if (tasks.add(1000, 0, true, 6, domeWrapper, "DomeMon")) {
    VLF("success");
    ready = true;
  } else { VLF("FAILED!"); }
}

// reset dome at the home position
void Dome::reset() {
  VLF("MSG: Dome, reset at home");
  axis1.resetPositionSteps(round(AXIS1_HOME_DEFAULT*AXIS1_STEPS_PER_DEGREE));
  axis1.enable(false);
  #if AXIS2_DRIVER_MODEL != OFF
    axis2.resetPositionSteps(round(AXIS2_HOME_DEFAULT*AXIS2_STEPS_PER_DEGREE));
    axis2.enable(false);
  #endif
  settings.park.state = PS_UNPARKED;
  nv.updateBytes(NV_DOME_SETTINGS_BASE, &settings, sizeof(DomeSettings));
}

// dome goto azimuth
CommandError Dome::gotoAzimuthTarget() {
  #if defined(ROOF_PRESENT) && DOME_SHUTTER_LOCK == ON
    if (!roof.open()) return CE_SLEW_ERR_IN_STANDBY;
  #endif
  if (settings.park.state >= PS_PARKED) return CE_SLEW_ERR_IN_PARK;

  VF("MSG: Dome, goto azimuth target coordinate set ("); V(targetAzm); VL("°)");
  VLF("MSG: Dome, starting azimuth goto");

  if (!axis1.isEnabled()) axis1.enable(true);
  axis1.setTargetCoordinate(targetAzm);

  if (axis1.isSlewing()) return CE_NONE;

  CommandError e = axis1.autoGoto(AXIS1_SLEW_RATE_DESIRED);
  return e;
}

// dome sync azimuth
CommandError Dome::syncAzimuthTarget() {
  #if defined(ROOF_PRESENT) && DOME_SHUTTER_LOCK == ON
    if (!roof.open()) return CE_SLEW_ERR_IN_STANDBY;
  #endif
  if (settings.park.state >= PS_PARKED) return CE_SLEW_ERR_IN_PARK;

  VF("MSG: Dome, sync azimuth target coordinate set ("); V(targetAzm); VL("°)");
  reset();
  if (!axis1.isEnabled()) axis1.enable(true);
  axis1.setInstrumentCoordinate(targetAzm);

  return CE_NONE;
}

// dome goto altitude
CommandError Dome::gotoAltitudeTarget() {
  #if AXIS2_DRIVER_MODEL != OFF
    #if defined(ROOF_PRESENT) && DOME_SHUTTER_LOCK == ON
      if (!roof.open()) return CE_SLEW_ERR_IN_STANDBY;
    #endif
    if (settings.park.state >= PS_PARKED) return CE_SLEW_ERR_IN_PARK;

    VF("MSG: Dome, goto altitude target coordinate set ("); V(targetAlt); VL("°)");
    VLF("MSG: Dome, starting altitude goto");

    if (!axis2.isEnabled()) axis2.enable(true);
    axis2.setTargetCoordinate(targetAlt);

    if (axis2.isSlewing()) return CE_NONE;

    CommandError e = axis2.autoGoto(AXIS2_SLEW_RATE_DESIRED);
    return e;
  #else
    return CE_GOTO_FAIL;
  #endif
}

// move to the home position
CommandError Dome::findHome() {
  #if defined(ROOF_PRESENT) && DOME_SHUTTER_LOCK == ON
    if (!roof.open()) return CE_SLEW_ERR_IN_STANDBY;
  #endif
  if (settings.park.state >= PS_PARKED) return CE_PARKED;

  targetAzm = AXIS1_HOME_DEFAULT;
  targetAlt = AXIS2_HOME_DEFAULT;

  CommandError e = CE_NONE;

  #if AXIS1_SENSE_HOME != OFF
    axis1.setFrequencySlew(AXIS1_SLEW_RATE_DESIRED);
    e = axis1.autoSlewHome();
    if (e == CE_NONE) homing = true;
  #else
    e = gotoAzimuthTarget();
  #endif

  #if AXIS2_DRIVER_MODEL != OFF
    if (e == CE_NONE) {
      #if AXIS2_SENSE_HOME != OFF
        axis2.setFrequencySlew(AXIS2_SLEW_RATE_DESIRED);
        e = axis2.autoSlewHome();
        if (e == CE_NONE) {
          homing = true;
        } else {
          axis1.autoSlewAbort();
          homing = false;
        }
      #else
        e = gotoAltitudeTarget();
      #endif
    }
  #endif


  return e;
}

// stop slew
void Dome::stop() {
  if (isSlewing()) {
    if (settings.park.state == PS_PARKING) {
      settings.park.state = PS_UNPARKED;
      nv.updateBytes(NV_DOME_SETTINGS_BASE, &settings, sizeof(DomeSettings));
    }
    axis1.autoSlewAbort();
    #if AXIS2_DRIVER_MODEL != OFF
      axis2.autoSlewAbort();
    #endif
    homing = false;
  }
}

// dome park
CommandError Dome::park() {
  #if defined(ROOF_PRESENT) && DOME_SHUTTER_LOCK == ON
    if (!roof.open()) return CE_SLEW_ERR_IN_STANDBY;
  #endif
  if (settings.park.state == PS_PARKED)      return CE_NONE;
  if (settings.park.state == PS_PARKING)     return CE_PARK_FAILED;
  if (settings.park.state == PS_UNPARKING)   return CE_PARK_FAILED;
  if (settings.park.state == PS_PARK_FAILED) return CE_PARK_FAILED;

  VLF("MSG: Dome, parking");
  axis1.enable(true);
  axis1.setBacklash(0.0F);
  axis1.setTargetCoordinatePark(settings.park.azimuth);
  CommandError e = axis1.autoGoto(AXIS1_SLEW_RATE_DESIRED);

  #if AXIS2_DRIVER_MODEL != OFF
    if (e == CE_NONE) {
      axis2.setBacklash(0.0F);
      axis2.setTargetCoordinatePark(settings.park.altitude);
      e = axis2.autoGoto(AXIS2_SLEW_RATE_DESIRED);
    }
  #endif

  if (e == CE_NONE) {
    settings.park.state = PS_PARKING;
    nv.updateBytes(NV_DOME_SETTINGS_BASE, &settings, sizeof(DomeSettings));
  }

  return e;
}

// dome unpark
CommandError Dome::unpark() {
  #if defined(ROOF_PRESENT) && DOME_SHUTTER_LOCK == ON
    if (!roof.open()) return CE_SLEW_ERR_IN_STANDBY;
  #endif
  if (settings.park.state == PS_PARKING)     return CE_PARK_FAILED;
  if (settings.park.state == PS_UNPARKING)   return CE_PARK_FAILED;
  if (settings.park.state == PS_PARK_FAILED) return CE_PARK_FAILED;

  VF("MSG: Dome, unpark position (Azm="); V(settings.park.azimuth); V("°, Alt="); V(settings.park.altitude); VL(")");

  axis1.enable(true);
  axis1.setBacklash(0.0F);
  axis1.setInstrumentCoordinatePark(settings.park.azimuth);
  axis1.setBacklash(settings.backlash.azimuth);
  axis1.setTargetCoordinate(settings.park.azimuth);
  CommandError e = axis1.autoGoto(AXIS1_SLEW_RATE_DESIRED);

  #if AXIS2_DRIVER_MODEL != OFF
    if (e == CE_NONE) {
      axis2.enable(true);
      axis2.setBacklash(0.0F);
      axis2.setInstrumentCoordinatePark(settings.park.altitude);
      axis2.setBacklash(settings.backlash.altitude);
      axis2.setTargetCoordinate(settings.park.altitude);
      e = axis2.autoGoto(AXIS2_SLEW_RATE_DESIRED);
    }
  #endif

  // wait for unpark to finish
  unsigned long timeout = millis() + 500;
  while (
    (axis1.isSlewing()
    #if AXIS2_DRIVER_MODEL != OFF
      || axis2.isSlewing()
    #endif
    ) && (long)(millis() - timeout) < 0
  ) { };

  if (e == CE_NONE) {
    settings.park.state = PS_UNPARKED;
    nv.updateBytes(NV_DOME_SETTINGS_BASE, &settings, sizeof(DomeSettings));
  }

  return e;
}

// dome set park
CommandError Dome::setpark() {
  #if defined(ROOF_PRESENT) && DOME_SHUTTER_LOCK == ON
    if (!roof.open()) return CE_SLEW_ERR_IN_STANDBY;
  #endif
  if (settings.park.state == PS_PARKED)      return CE_NONE;
  if (settings.park.state == PS_PARKING)     return CE_PARK_FAILED;
  if (settings.park.state == PS_UNPARKING)   return CE_PARK_FAILED;
  if (settings.park.state == PS_PARK_FAILED) return CE_PARK_FAILED;

  VLF("MSG: Dome, setting park position");
  settings.park.azimuth = axis1.getInstrumentCoordinate();
  #if AXIS2_DRIVER_MODEL != OFF
    settings.park.altitude = axis2.getInstrumentCoordinate();
  #else
    settings.park.altitude = NAN;
  #endif

  nv.updateBytes(NV_DOME_SETTINGS_BASE, &settings, sizeof(DomeSettings));
  return CE_NONE;
}

// check if dome is slewing
bool Dome::isSlewing() {
  return
    (axis1.isSlewing()
    #if AXIS2_DRIVER_MODEL != OFF
      || axis2.isSlewing()
    #endif
    );
}

// check if dome is parked
bool Dome::isParked() {
  return settings.park.state == PS_PARKED;
}

// return dome status message
const char* Dome::statusMessage() {
  if (settings.park.state == PS_PARKED) strcpy(statusMsg, L_PARKED); else
  if (settings.park.state == PS_PARKING) strcpy(statusMsg, L_PARKING); else
  if (isSlewing()) strcpy(statusMsg, L_SLEWING); else strcpy(statusMsg, L_STOPPED);
  return statusMsg;
}

// poll dome to monitor motion
void Dome::monitor() {
  if (!axis1.isSlewing()) {
  #if AXIS2_DRIVER_MODEL != OFF
    if (!axis2.isSlewing())
  #endif
    {
      if (settings.park.state == PS_PARKING) {
        settings.park.state = PS_PARKED;
        nv.updateBytes(NV_DOME_SETTINGS_BASE, &settings, sizeof(DomeSettings));
        axis1.enable(false);
        #if AXIS2_DRIVER_MODEL != OFF
          axis2.enable(false);
        #endif
      } else

      if (settings.park.state == PS_UNPARKED) {
        if (homing) {
          reset();
          homing = false;
        }
      }
    }
  }
}

Dome dome;

#endif
