// Dome control

#include "Dome.h"

#ifdef DOME_PRESENT
  #ifdef AXIS1_SERVO_PRESENT
    ServoControl servoControlAxis1;

    #if AXIS1_ENCODER == AB
      Quadrature encAxis1(AXIS1_ENCODER_A_PIN, AXIS1_ENCODER_B_PIN, 1);
    #elif AXIS1_ENCODER == CW_CCW
      CwCcw encAxis1(AXIS1_ENCODER_A_PIN, AXIS1_ENCODER_B_PIN, 1);
    #elif AXIS1_ENCODER == PULSE_DIR
      PulseDir encAxis1(AXIS1_ENCODER_A_PIN, AXIS1_ENCODER_B_PIN, 1);
    #elif AXIS1_ENCODER == PULSE_ONLY
      PulseOnly encAxis1(AXIS1_ENCODER_A_PIN, &servoControlAxis1.directionHint, 1);
    #elif AXIS1_ENCODER == VIRTUAL
      VirtualEnc encAxis1(1);
    #elif AXIS1_ENCODER == AS37_H39B_B
      As37h39bb encAxis1(AXIS1_ENCODER_A_PIN, AXIS1_ENCODER_B_PIN, 1);
    #elif AXIS1_ENCODER == SERIAL_BRIDGE
      SerialBridge encAxis1(1);
    #endif

    #if AXIS1_SERVO_FLTR == KALMAN
      KalmanFilter filterAxis1(AXIS1_SERVO_FLTR_MEAS_U, AXIS1_SERVO_FLTR_VARIANCE);
    #elif AXIS1_SERVO_FLTR == ROLLING
      RollingFilter filterAxis1(AXIS1_SERVO_FLTR_WSIZE);
    #elif AXIS1_SERVO_FLTR == WINDOWING
      WindowingFilter filterAxis1(AXIS1_SERVO_FLTR_WSIZE);
    #elif AXIS1_SERVO_FLTR == OFF
      Filter filterAxis1;
    #endif

    #if AXIS1_SERVO_FEEDBACK == FB_PID
      Pid pidAxis1(AXIS1_PID_P, AXIS1_PID_I, AXIS1_PID_D, AXIS1_PID_P_GOTO, AXIS1_PID_I_GOTO, AXIS1_PID_D_GOTO);
    #endif

    #if AXIS1_DRIVER_MODEL == SERVO_EE || AXIS1_DRIVER_MODEL == SERVO_PE
      const ServoDcPins ServoPinsAxis1 = {AXIS1_SERVO_PH1_PIN, AXIS1_SERVO_PH1_STATE, AXIS1_SERVO_PH2_PIN, AXIS1_SERVO_PH2_STATE, AXIS1_ENABLE_PIN, AXIS1_ENABLE_STATE, AXIS1_FAULT_PIN};
      const ServoDcSettings ServoSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_STATUS, AXIS1_SERVO_VELOCITY_MAX, AXIS1_SERVO_ACCELERATION};
      ServoDc driver1(1, &ServoPinsAxis1, &ServoSettingsAxis1);
    #endif
    ServoMotor motor1(1, ((ServoDriver*)&driver1), &filterAxis1, &encAxis1, AXIS1_ENCODER_ORIGIN, AXIS1_ENCODER_REVERSE == ON, &pidAxis1, &servoControlAxis1, AXIS1_SYNC_THRESHOLD);
  #endif

  #ifdef AXIS1_DRIVER_PRESENT
    const StepDirDriverPins DriverPinsAxis1 = {AXIS1_M0_PIN, AXIS1_M1_PIN, AXIS1_M2_PIN, AXIS1_M2_ON_STATE, AXIS1_M3_PIN, AXIS1_DECAY_PIN, AXIS1_FAULT_PIN};
    const StepDirDriverSettings DriverSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_MICROSTEPS, AXIS1_DRIVER_MICROSTEPS_GOTO, AXIS1_DRIVER_IHOLD, AXIS1_DRIVER_IRUN, AXIS1_DRIVER_IGOTO, AXIS1_DRIVER_DECAY, AXIS1_DRIVER_DECAY_GOTO, AXIS1_DRIVER_STATUS};
    #if defined(AXIS1_STEP_DIR_LEGACY)
      StepDirGeneric driver1(1, &DriverPinsAxis1, &DriverSettingsAxis1);
    #endif
    const StepDirPins StepDirPinsAxis1 = {AXIS1_STEP_PIN, AXIS1_STEP_STATE, AXIS1_DIR_PIN, AXIS1_ENABLE_PIN, AXIS1_ENABLE_STATE};
    StepDirMotor motor1(1, &StepDirPinsAxis1, ((StepDirDriver*)&driver1));
  #endif

  const AxisPins PinsAxis1 = {AXIS1_SENSE_LIMIT_MIN_PIN, AXIS1_SENSE_HOME_PIN, AXIS1_SENSE_LIMIT_MAX_PIN, {AXIS1_SENSE_HOME, AXIS1_SENSE_HOME_INIT, AXIS1_SENSE_HOME_DIST_LIMIT, AXIS1_SENSE_LIMIT_MIN, AXIS1_SENSE_LIMIT_MAX, AXIS1_SENSE_LIMIT_INIT}};
  const AxisSettings SettingsAxis1 = {AXIS1_STEPS_PER_DEGREE, AXIS1_REVERSE, {AXIS1_LIMIT_MIN, AXIS1_LIMIT_MAX}, AXIS1_BACKLASH_RATE};
  #ifdef AXIS1_TARGET_TOLERANCE
    Axis axis1(1, &PinsAxis1, &SettingsAxis1, AXIS_MEASURE_DEGREES, AXIS1_TARGET_TOLERANCE);
  #else
    Axis axis1(1, &PinsAxis1, &SettingsAxis1, AXIS_MEASURE_DEGREES);
  #endif

  #if AXIS2_DRIVER_MODEL != OFF
    #ifdef AXIS2_SERVO_PRESENT
      ServoControl servoControlAxis2;

      #if AXIS2_ENCODER == AB
        Quadrature encAxis2(AXIS2_ENCODER_A_PIN, AXIS2_ENCODER_B_PIN, 2);
      #elif AXIS2_ENCODER == CW_CCW
        CwCcw encAxis2(AXIS2_ENCODER_A_PIN, AXIS2_ENCODER_B_PIN, 2);
      #elif AXIS2_ENCODER == PULSE_DIR
        PulseDir encAxis2(AXIS2_ENCODER_A_PIN, AXIS2_ENCODER_B_PIN, 2);
      #elif AXIS2_ENCODER == PULSE_ONLY
        PulseOnly encAxis2(AXIS2_ENCODER_A_PIN, &servoControlAxis2.directionHint, 2);
      #elif AXIS1_ENCODER == VIRTUAL
        VirtualEnc encAxis1(1);
      #elif AXIS2_ENCODER == AS37_H39B_B
        As37h39bb encAxis2(AXIS2_ENCODER_A_PIN, AXIS2_ENCODER_B_PIN, 2);
      #elif AXIS2_ENCODER == SERIAL_BRIDGE
        SerialBridge encAxis2(2);
      #endif

      #if AXIS2_SERVO_FEEDBACK == FB_PID
        Pid pidAxis2(AXIS2_PID_P, AXIS2_PID_I, AXIS2_PID_D, AXIS2_PID_P_GOTO, AXIS2_PID_I_GOTO, AXIS2_PID_D_GOTO);
      #endif

      #if AXIS2_SERVO_FLTR == KALMAN
        KalmanFilter filterAxis2(AXIS2_SERVO_FLTR_MEAS_U, AXIS2_SERVO_FLTR_VARIANCE);
      #elif AXIS2_SERVO_FLTR == ROLLING
        RollingFilter filterAxis2(AXIS2_SERVO_FLTR_WSIZE);
      #elif AXIS2_SERVO_FLTR == WINDOWING
        WindowingFilter filterAxis2(AXIS2_SERVO_FLTR_WSIZE);
      #elif AXIS2_SERVO_FLTR == OFF
        Filter filterAxis2;
      #endif

      #if AXIS2_DRIVER_MODEL == SERVO_EE || AXIS2_DRIVER_MODEL == SERVO_PE
        const ServoDcPins ServoPinsAxis2 = {AXIS2_SERVO_PH1_PIN, AXIS2_SERVO_PH1_STATE, AXIS2_SERVO_PH2_PIN, AXIS2_SERVO_PH2_STATE, AXIS2_ENABLE_PIN, AXIS2_ENABLE_STATE, AXIS2_FAULT_PIN};
        const ServoDcSettings ServoSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_STATUS, AXIS2_SERVO_VELOCITY_MAX, AXIS2_SERVO_ACCELERATION};
        ServoDc driver2(2, &ServoPinsAxis2, &ServoSettingsAxis2);
      #endif
      ServoMotor motor2(2, ((ServoDriver*)&driver2), &filterAxis2, &encAxis2, AXIS2_ENCODER_ORIGIN, AXIS2_ENCODER_REVERSE == ON, &pidAxis2, &servoControlAxis2, AXIS2_SYNC_THRESHOLD);
    #endif

    #ifdef AXIS2_DRIVER_PRESENT
      const StepDirDriverPins DriverPinsAxis2 = {AXIS2_M0_PIN, AXIS2_M1_PIN, AXIS2_M2_PIN, AXIS2_M2_ON_STATE, AXIS2_M3_PIN, AXIS2_DECAY_PIN, AXIS2_FAULT_PIN};
      const StepDirDriverSettings DriverSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_MICROSTEPS, AXIS2_DRIVER_MICROSTEPS_GOTO, AXIS2_DRIVER_IHOLD, AXIS2_DRIVER_IRUN, AXIS2_DRIVER_IGOTO, AXIS2_DRIVER_DECAY, AXIS2_DRIVER_DECAY_GOTO, AXIS2_DRIVER_STATUS};
      #if defined(AXIS2_STEP_DIR_LEGACY)
        StepDirGeneric driver2(2, &DriverPinsAxis2, &DriverSettingsAxis2);
      #endif
      const StepDirPins StepDirPinsAxis2 = {AXIS2_STEP_PIN, AXIS2_STEP_STATE, AXIS2_DIR_PIN, AXIS2_ENABLE_PIN, AXIS2_ENABLE_STATE};
      StepDirMotor motor2(2, &StepDirPinsAxis2, ((StepDirDriver*)&driver2));
    #endif

    const AxisPins PinsAxis2 = {AXIS2_SENSE_LIMIT_MIN_PIN, AXIS2_SENSE_HOME_PIN, AXIS2_SENSE_LIMIT_MAX_PIN, {AXIS2_SENSE_HOME, AXIS2_SENSE_HOME_INIT, AXIS2_SENSE_HOME_DIST_LIMIT, AXIS2_SENSE_LIMIT_MIN, AXIS2_SENSE_LIMIT_MAX, AXIS2_SENSE_LIMIT_INIT}};
    const AxisSettings SettingsAxis2 = {AXIS2_STEPS_PER_DEGREE, AXIS2_REVERSE, {AXIS2_LIMIT_MIN, AXIS2_LIMIT_MAX}, AXIS2_BACKLASH_RATE};
    Axis axis2(2, &PinsAxis2, &SettingsAxis2, AXIS_MEASURE_DEGREES);

  #endif
#endif
