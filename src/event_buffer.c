/*
 * Copyright (c) 2026 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_buffer.h"

/* Fixed-capacity ring of queued events. One slot is always left empty so that
 * front == rear means "empty" without a separate count. */

void event_loop_queue_init(EventLoopQueue *q, EventLoopQueueElm *data, uint16_t len)
{
    q->data = data;
    q->front = 0;
    q->rear = 0;
    q->len = (int16_t)len;
}

EventLoopQueueElm event_loop_queue_pop(EventLoopQueue *q)
{
    if (q->front == q->rear)
    {
        const EventLoopQueueElm empty = {{0, 0, 0, NULL}, 0};
        return empty;
    }
    const EventLoopQueueElm element = q->data[q->front];
    q->front = event_loop_queue_next_index(q, q->front);
    return element;
}

void event_loop_queue_push(EventLoopQueue *q, EventLoopQueueElm t)
{
    const int16_t next_rear = event_loop_queue_next_index(q, q->rear);
    if (next_rear == q->front)
    {
        return; /* full: the newest event is dropped */
    }
    q->data[q->rear] = t;
    q->rear = next_rear;
}
