/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef ANALOG_INTERNAL_H_
#define ANALOG_INTERNAL_H_

#include "analog.h"

/* Share the window read with the default raw-sample hook, without another
 * function call for every key. Dirty windows retain the full data scan. */
static inline AnalogRawValue ringbuf_average(RingBuffer* ringbuf)
{
#ifdef OPTIMIZE_MOVING_AVERAGE_FOR_RINGBUF
    if (!ringbuf->dirty)
    {
        return (AnalogRawValue)(ringbuf->sum / RING_BUF_LEN);
    }
#endif
    uint32_t sum = 0;
    for (int i = 0; i < RING_BUF_LEN; i++)
    {
        sum += ringbuf->datas[i];
    }
    return (AnalogRawValue)(sum / RING_BUF_LEN);
}

#endif /* ANALOG_INTERNAL_H_ */
