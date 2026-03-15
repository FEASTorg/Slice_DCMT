// src/main.cpp
/**
 * @file main.cpp
 * @brief DCMT firmware (PlatformIO) for the BREAD system.
 */

#include <Arduino.h>
#include <FastLED.h>
#include <crumbs.h>
#include <crumbs_arduino.h>
#include <LMD18200.h>
#include <bread/dcmt_ops.h>
#include <Encoder.h>

#include "config.h"
#include "config_hardware.h"
#include "globals.h"

// ---- CRUMBS context ----
static crumbs_context_t ctx;

// ---- Global flags/objects ----
volatile bool estopTriggered = false;
CRGB led;

// Motor drivers
LMD18200 motor1Driver(MOTOR1_PWM_PIN, MOTOR1_DIR_PIN, MOTOR1_BRAKE_PIN, MOTOR1_CSENSE_PIN);
LMD18200 motor2Driver(MOTOR2_PWM_PIN, MOTOR2_DIR_PIN, MOTOR2_BRAKE_PIN, MOTOR2_CSENSE_PIN);

Encoder motor1Encoder(MOTOR1_ENCODER_PIN1, MOTOR1_ENCODER_PIN2);
Encoder motor2Encoder(MOTOR2_ENCODER_PIN1, MOTOR2_ENCODER_PIN2);
static long lastSpeedPos1 = 0;
static long lastSpeedPos2 = 0;

// Shared state
DCMT_SLICE slice;
Timing timing = {0};

static int16_t clamp_pwm(long v)
{
    if (v < -255)
        return -255;
    if (v > 255)
        return 255;
    return static_cast<int16_t>(v);
}

static int16_t clamp_i16(long v)
{
    if (v < -32768)
        return -32768;
    if (v > 32767)
        return 32767;
    return static_cast<int16_t>(v);
}

static int16_t pid_step(const PIDTunings &pid, PIDState &state, float error, float dt_s)
{
    if (dt_s <= 0.0f)
        dt_s = 0.02f;

    state.integral += error * dt_s;
    if (state.integral > 600.0f)
        state.integral = 600.0f;
    if (state.integral < -600.0f)
        state.integral = -600.0f;

    const float derivative = (error - state.previousError) / dt_s;
    state.previousError = error;

    const float output = pid.kp * error + pid.ki * state.integral + pid.kd * derivative;
    return clamp_pwm(static_cast<long>(output));
}

static void update_speed_measurement(uint32_t now_ms)
{
    if ((now_ms - timing.lastSpeedSample) < SPEED_SAMPLE_TIME_MS)
        return;

    const uint32_t dt_ms = static_cast<uint32_t>(now_ms - timing.lastSpeedSample);
    const long pos1 = motor1Encoder.read();
    const long pos2 = motor2Encoder.read();

    const long d1 = pos1 - lastSpeedPos1;
    const long d2 = pos2 - lastSpeedPos2;

    slice.motor1Speed = clamp_i16((d1 * 1000L) / static_cast<long>(dt_ms));
    slice.motor2Speed = clamp_i16((d2 * 1000L) / static_cast<long>(dt_ms));

    lastSpeedPos1 = pos1;
    lastSpeedPos2 = pos2;
    timing.lastSpeedSample = now_ms;
}

static void run_closed_loop(uint32_t now_ms)
{
    if ((now_ms - timing.lastControlUpdate) < CONTROL_UPDATE_TIME_MS)
        return;

    const uint32_t dt_ms = static_cast<uint32_t>(now_ms - timing.lastControlUpdate);
    const float dt_s = static_cast<float>(dt_ms) / 1000.0f;

    const long pos1 = motor1Encoder.read();
    const long pos2 = motor2Encoder.read();
    slice.motor1Position = clamp_i16(pos1);
    slice.motor2Position = clamp_i16(pos2);

    update_speed_measurement(now_ms);

    if (slice.mode == CLOSED_LOOP_POSITION)
    {
        const float err1 = static_cast<float>(slice.motor1PositionSetpoint - slice.motor1Position);
        const float err2 = static_cast<float>(slice.motor2PositionSetpoint - slice.motor2Position);
        slice.motor1PWM = pid_step(slice.posPid1, slice.posState1, err1, dt_s);
        slice.motor2PWM = pid_step(slice.posPid2, slice.posState2, err2, dt_s);
    }
#if DCMT_ENABLE_SPEED_LOOP
    else if (slice.mode == CLOSED_LOOP_SPEED)
    {
        const float err1 = static_cast<float>(slice.motor1SpeedSetpoint - slice.motor1Speed);
        const float err2 = static_cast<float>(slice.motor2SpeedSetpoint - slice.motor2Speed);
        slice.motor1PWM = pid_step(slice.speedPid1, slice.speedState1, err1, dt_s);
        slice.motor2PWM = pid_step(slice.speedPid2, slice.speedState2, err2, dt_s);
    }
#endif

    timing.lastControlUpdate = now_ms;
}

void setup()
{
    setupSlice();
    setupDCMT();
    delay(1000);
}

void loop()
{
    pollEStop();
    motorControlLogic();
    serialCommands();
    printSerialOutput();
}

// ---- Implementation (previously in .ino files) ----

void setupSlice()
{
    int rc;
    Serial.begin(115200);

    // CRUMBS
    crumbs_arduino_init_peripheral(&ctx, I2C_ADR);

    rc = crumbs_register_handler(&ctx, DCMT_OP_SET_OPEN_LOOP, handler_set_open_loop, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_SET_OPEN_LOOP"));

    rc = crumbs_register_handler(&ctx, DCMT_OP_SET_BRAKE, handler_set_brake, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_SET_BRAKE"));

    rc = crumbs_register_handler(&ctx, DCMT_OP_SET_MODE, handler_set_mode, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_SET_MODE"));

    rc = crumbs_register_handler(&ctx, DCMT_OP_SET_SETPOINT, handler_set_setpoint, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_SET_SETPOINT"));

    rc = crumbs_register_handler(&ctx, DCMT_OP_SET_PID, handler_set_pid, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_SET_PID"));

    rc = crumbs_register_reply_handler(&ctx, 0x00, reply_version, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register version reply handler"));

    rc = crumbs_register_reply_handler(&ctx, DCMT_OP_GET_STATE, reply_get_state, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_GET_STATE reply handler"));

    rc = crumbs_register_reply_handler(&ctx, BREAD_OP_GET_CAPS, reply_get_caps, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register BREAD_OP_GET_CAPS reply handler"));

    // LED
    FastLED.addLeds<NEOPIXEL, LED_PIN>(&led, 1);
    FastLED.setBrightness(50);
    led = CRGB::Blue;
    FastLED.show();

    // e-stop
    pinMode(ESTOP, INPUT);
    attachInterrupt(digitalPinToInterrupt(ESTOP), estopISR, CHANGE);

    SLICE_DEBUG_PRINTLN(F("DCMT SLICE INITIALIZED"));
    SLICE_DEBUG_PRINTLN(F("VERSION: " VERSION));
#if (DCMT_HW_GEN == 1)
    SLICE_DEBUG_PRINTLN(F("HW Profile: Gen1"));
#else
    SLICE_DEBUG_PRINTLN(F("HW Profile: Gen2"));
#endif
    SLICE_DEBUG_PRINTLN(F("Control Profile: Open + Closed Position"));
#if DCMT_ENABLE_SPEED_LOOP
    SLICE_DEBUG_PRINTLN(F("Speed Profile: Enabled"));
#else
    SLICE_DEBUG_PRINTLN(F("Speed Profile: Disabled"));
#endif
}

void setupDCMT()
{
    motor1Driver.begin();
    motor2Driver.begin();
    motor1Driver.write(0);
    motor2Driver.write(0);

    timing.lastSerialPrint = millis();
    timing.lastControlUpdate = millis();
    timing.lastSpeedSample = millis();
    lastSpeedPos1 = motor1Encoder.read();
    lastSpeedPos2 = motor2Encoder.read();
}

// Poll/ISR/processing
void pollEStop()
{
    if (estopTriggered)
    {
        processEStop();
        estopTriggered = false;
    }

    if (slice.eStop)
    {
        led = CRGB::Red;
    }
    else
    {
        led = CRGB::Green;
    }
    FastLED.show();
}

void estopISR()
{
    estopTriggered = true;
}

void processEStop()
{
    if (digitalRead(ESTOP) == HIGH)
    {
        motor1Driver.brake();
        motor2Driver.brake();
        slice.eStop = true;
        slice.motor1PWM = 0;
        slice.motor2PWM = 0;
        SLICE_DEBUG_PRINTLN(F("ESTOP PRESSED!"));
    }
    else
    {
        slice.eStop = false;
        SLICE_DEBUG_PRINTLN(F("ESTOP RELEASED!"));
    }
}

void motorControlLogic()
{
    if (slice.eStop)
    {
        motor1Driver.write(0);
        motor2Driver.write(0);
        motor1Driver.brake();
        motor2Driver.brake();
        return;
    }

    if (slice.mode == CLOSED_LOOP_POSITION
#if DCMT_ENABLE_SPEED_LOOP
        || slice.mode == CLOSED_LOOP_SPEED
#endif
    )
    {
        run_closed_loop(static_cast<uint32_t>(millis()));
    }

    if (slice.motor1Brake)
    {
        slice.motor1PWM = 0;
        motor1Driver.write(0);
        motor1Driver.brake();
    }
    else
    {
        motor1Driver.write(clamp_pwm(slice.motor1PWM));
    }

    if (slice.motor2Brake)
    {
        slice.motor2PWM = 0;
        motor2Driver.write(0);
        motor2Driver.brake();
    }
    else
    {
        motor2Driver.write(clamp_pwm(slice.motor2PWM));
    }
}
