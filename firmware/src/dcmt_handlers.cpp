#include <Arduino.h>
#include <crumbs.h>
#include <crumbs_message_helpers.h>

#include <dcmt_ops.h>

#include "globals.h"

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
    crumbs_msg_add_u8(reply, 0x00); // mode: OPEN_LOOP (reserved for future)
    crumbs_msg_add_i16(reply, slice.motor1PWM);
    crumbs_msg_add_i16(reply, slice.motor2PWM);
    crumbs_msg_add_u8(reply, brakes);
    crumbs_msg_add_u8(reply, slice.eStop ? 1 : 0);
}
