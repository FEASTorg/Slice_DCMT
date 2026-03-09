#ifndef DCMT_OPS_H
#define DCMT_OPS_H

#include "crumbs.h"
#include "crumbs_message_helpers.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DCMT_TYPE_ID 0x02

#define DCMT_MODULE_VER_MAJOR 1
#define DCMT_MODULE_VER_MINOR 0
#define DCMT_MODULE_VER_PATCH 0

#define DCMT_OP_SET_OPEN_LOOP 0x01
#define DCMT_OP_SET_BRAKE 0x02

#define DCMT_OP_GET_STATE 0x80

static inline int dcmt_send_set_open_loop(const crumbs_device_t *dev, int16_t m1_pwm, int16_t m2_pwm)
{
    crumbs_message_t msg;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_OPEN_LOOP);
    crumbs_msg_add_i16(&msg, m1_pwm);
    crumbs_msg_add_i16(&msg, m2_pwm);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_brake(const crumbs_device_t *dev, uint8_t m1_brake, uint8_t m2_brake)
{
    crumbs_message_t msg;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_BRAKE);
    crumbs_msg_add_u8(&msg, m1_brake);
    crumbs_msg_add_u8(&msg, m2_brake);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_query_state(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, DCMT_OP_GET_STATE);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

typedef struct
{
    uint8_t mode;
    int16_t m1_pwm;
    int16_t m2_pwm;
    uint8_t brakes;
    uint8_t estop;
} dcmt_state_result_t;

static inline int dcmt_get_state(const crumbs_device_t *dev, dcmt_state_result_t *out)
{
    crumbs_message_t reply;
    int rc;

    if (!out)
        return -1;

    rc = dcmt_query_state(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != DCMT_TYPE_ID || reply.opcode != DCMT_OP_GET_STATE)
        return -1;

    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 0, &out->mode);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 1, &out->m1_pwm);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 3, &out->m2_pwm);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 5, &out->brakes);
    if (rc != 0)
        return rc;
    return crumbs_msg_read_u8(reply.data, reply.data_len, 6, &out->estop);
}

#ifdef __cplusplus
}
#endif

#endif // DCMT_OPS_H
