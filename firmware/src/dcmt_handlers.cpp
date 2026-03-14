#include <Arduino.h>
#include <crumbs.h>
#include <crumbs_message_helpers.h>

#include <bread/dcmt_ops.h>

#include "globals.h"

static bool is_valid_mode(uint8_t mode)
{
    return (mode == OPEN_LOOP) || (mode == CLOSED_LOOP_POSITION) || (mode == CLOSED_LOOP_SPEED);
}

void handler_set_open_loop(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data)
{
    int16_t m1 = 0;
    int16_t m2 = 0;
    (void)ctx;
    (void)opcode;
    (void)user_data;

    if (crumbs_msg_read_i16(data, data_len, 0, &m1) != 0)
        return;
    if (crumbs_msg_read_i16(data, data_len, 2, &m2) != 0)
        return;

#if DCMT_FEATURE_CLOSED_LOOP
    if (slice.mode != OPEN_LOOP)
        return;
#endif

    slice.motor1PWM = constrain(m1, -255, 255);
    slice.motor2PWM = constrain(m2, -255, 255);
}

void handler_set_brake(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data)
{
    uint8_t b1 = 0;
    uint8_t b2 = 0;
    (void)ctx;
    (void)opcode;
    (void)user_data;

    if (crumbs_msg_read_u8(data, data_len, 0, &b1) != 0)
        return;
    if (crumbs_msg_read_u8(data, data_len, 1, &b2) != 0)
        return;

    slice.motor1Brake = (b1 != 0);
    slice.motor2Brake = (b2 != 0);
}

void handler_set_mode(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data)
{
    uint8_t mode = OPEN_LOOP;
    (void)ctx;
    (void)opcode;
    (void)user_data;

    if (crumbs_msg_read_u8(data, data_len, 0, &mode) != 0)
        return;

#if DCMT_FEATURE_CLOSED_LOOP
    if (!is_valid_mode(mode))
        return;
    slice.mode = static_cast<ControlModes>(mode);
    if (slice.mode != OPEN_LOOP)
    {
        slice.motor1PWM = 0;
        slice.motor2PWM = 0;
        slice.posState1 = {};
        slice.posState2 = {};
        slice.speedState1 = {};
        slice.speedState2 = {};
    }
#else
    (void)mode;
    slice.mode = OPEN_LOOP;
#endif
}

void handler_set_setpoint(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data)
{
    int16_t t1 = 0;
    int16_t t2 = 0;
    (void)ctx;
    (void)opcode;
    (void)user_data;

    if (crumbs_msg_read_i16(data, data_len, 0, &t1) != 0)
        return;
    if (crumbs_msg_read_i16(data, data_len, 2, &t2) != 0)
        return;

#if DCMT_FEATURE_CLOSED_LOOP
    if (slice.mode == CLOSED_LOOP_POSITION)
    {
        slice.motor1PositionSetpoint = t1;
        slice.motor2PositionSetpoint = t2;
        return;
    }
    if (slice.mode == CLOSED_LOOP_SPEED)
    {
        slice.motor1SpeedSetpoint = t1;
        slice.motor2SpeedSetpoint = t2;
    }
#else
    (void)t1;
    (void)t2;
#endif
}

void handler_set_pid(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data)
{
    uint8_t kp1_x10 = 0;
    uint8_t ki1_x10 = 0;
    uint8_t kd1_x10 = 0;
    uint8_t kp2_x10 = 0;
    uint8_t ki2_x10 = 0;
    uint8_t kd2_x10 = 0;
    (void)ctx;
    (void)opcode;
    (void)user_data;

    if (crumbs_msg_read_u8(data, data_len, 0, &kp1_x10) != 0)
        return;
    if (crumbs_msg_read_u8(data, data_len, 1, &ki1_x10) != 0)
        return;
    if (crumbs_msg_read_u8(data, data_len, 2, &kd1_x10) != 0)
        return;
    if (crumbs_msg_read_u8(data, data_len, 3, &kp2_x10) != 0)
        return;
    if (crumbs_msg_read_u8(data, data_len, 4, &ki2_x10) != 0)
        return;
    if (crumbs_msg_read_u8(data, data_len, 5, &kd2_x10) != 0)
        return;

#if DCMT_FEATURE_CLOSED_LOOP
    const float kp1 = static_cast<float>(kp1_x10) / 10.0f;
    const float ki1 = static_cast<float>(ki1_x10) / 10.0f;
    const float kd1 = static_cast<float>(kd1_x10) / 10.0f;
    const float kp2 = static_cast<float>(kp2_x10) / 10.0f;
    const float ki2 = static_cast<float>(ki2_x10) / 10.0f;
    const float kd2 = static_cast<float>(kd2_x10) / 10.0f;

    if (slice.mode == CLOSED_LOOP_POSITION)
    {
        slice.posPid1 = {kp1, ki1, kd1};
        slice.posPid2 = {kp2, ki2, kd2};
        return;
    }
    if (slice.mode == CLOSED_LOOP_SPEED)
    {
        slice.speedPid1 = {kp1, ki1, kd1};
        slice.speedPid2 = {kp2, ki2, kd2};
    }
#else
    (void)kp1_x10;
    (void)ki1_x10;
    (void)kd1_x10;
    (void)kp2_x10;
    (void)ki2_x10;
    (void)kd2_x10;
#endif
}

void reply_version(crumbs_context_t *ctx, crumbs_message_t *reply, void *user_data)
{
    (void)ctx;
    (void)user_data;
    crumbs_build_version_reply(reply, DCMT_TYPE_ID, DCMT_MODULE_VER_MAJOR, DCMT_MODULE_VER_MINOR, DCMT_MODULE_VER_PATCH);
}

void reply_get_state(crumbs_context_t *ctx, crumbs_message_t *reply, void *user_data)
{
    uint8_t brakes = 0;
    (void)ctx;
    (void)user_data;

    if (slice.motor1Brake)
        brakes |= 0x01;
    if (slice.motor2Brake)
        brakes |= 0x02;

    crumbs_msg_init(reply, DCMT_TYPE_ID, DCMT_OP_GET_STATE);
    crumbs_msg_add_u8(reply, static_cast<uint8_t>(slice.mode));

#if DCMT_FEATURE_CLOSED_LOOP
    if (slice.mode == CLOSED_LOOP_POSITION)
    {
        crumbs_msg_add_i16(reply, slice.motor1PositionSetpoint);
        crumbs_msg_add_i16(reply, slice.motor2PositionSetpoint);
        crumbs_msg_add_i16(reply, slice.motor1Position);
        crumbs_msg_add_i16(reply, slice.motor2Position);
        crumbs_msg_add_u8(reply, brakes);
        crumbs_msg_add_u8(reply, slice.eStop ? 1 : 0);
        return;
    }
    if (slice.mode == CLOSED_LOOP_SPEED)
    {
        crumbs_msg_add_i16(reply, slice.motor1SpeedSetpoint);
        crumbs_msg_add_i16(reply, slice.motor2SpeedSetpoint);
        crumbs_msg_add_i16(reply, slice.motor1Speed);
        crumbs_msg_add_i16(reply, slice.motor2Speed);
        crumbs_msg_add_u8(reply, brakes);
        crumbs_msg_add_u8(reply, slice.eStop ? 1 : 0);
        return;
    }
#endif

    crumbs_msg_add_i16(reply, slice.motor1PWM);
    crumbs_msg_add_i16(reply, slice.motor2PWM);
    crumbs_msg_add_u8(reply, brakes);
    crumbs_msg_add_u8(reply, slice.eStop ? 1 : 0);
}

void reply_get_caps(crumbs_context_t *ctx, crumbs_message_t *reply, void *user_data)
{
    uint8_t level = DCMT_CAP_LEVEL_1;
    uint32_t flags = DCMT_CAP_BASELINE_FLAGS;
    (void)ctx;
    (void)user_data;

#if DCMT_FEATURE_CLOSED_LOOP
    level = DCMT_CAP_LEVEL_2;
    flags |= DCMT_CAP_CLOSED_LOOP_POSITION;
    flags |= DCMT_CAP_CLOSED_LOOP_SPEED;
    flags |= DCMT_CAP_PID_TUNING;
#endif

    (void)bread_caps_build_reply(reply, DCMT_TYPE_ID, level, flags);
}
