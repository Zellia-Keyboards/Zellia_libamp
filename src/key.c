/*
 * Copyright (c) 2025 Zellia-Keyboards
 * Original Author: Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "key.h"

void key_update(Key* key,bool state)
{
    if ((!(key->state)) && state)
    {
        if (key->key_cb[KEY_EVENT_DOWN])
            key->key_cb[KEY_EVENT_DOWN](key);
    }
    if ((key->state) && (!state))
    {
        if (key->key_cb[KEY_EVENT_UP])
            key->key_cb[KEY_EVENT_UP](key);
    }
    key->state = state;
}

void key_attach(Key* key, KEY_EVENT e, key_cb_t cb)
{
    key->key_cb[e] = cb;
}

float key_get_bottom_distance(Key *key)
{
    return key->bottom_distance;
}

void key_set_bottom_distance(Key *key, float distance)
{
    key->bottom_distance = distance;
}
