/*
 * Copyright (c) 2025 Zellia-Keyboards
 * Original Author: Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "keyboard.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JOYSTICK_MAX_VALUE ((1 << 7) - 1)

typedef int8_t JoystickAxis;

typedef struct __Joystick {
#ifdef JOYSTICK_SHARED_EP
    uint8_t report_id;
#endif
#if JOYSTICK_AXIS_COUNT > 0
    JoystickAxis axes[JOYSTICK_AXIS_COUNT];
#endif
#if JOYSTICK_BUTTON_COUNT > 0
    uint8_t buttons[(JOYSTICK_BUTTON_COUNT - 1) / 8 + 1];
#endif
} __PACKED Joystick;

#define JOYSTICK_KEYCODE_GET_AXIS_MAP(keycode) (KEYCODE_GET_SUB((keycode) >> 5) & 0x03)
#define JOYSTICK_KEYCODE_IS_AXIS_INVERT(keycode) (KEYCODE_GET_SUB((keycode)) & 0x80)
#define JOYSTICK_KEYCODE_IS_AXIS(keycode) (KEYCODE_GET_SUB((keycode)) & 0xE0)
#define JOYSTICK_KEYCODE_GET_AXIS_INDEX(keycode) (KEYCODE_GET_SUB((keycode)) & 0x1F)

void joystick_event_handler(KeyboardEvent event);
void joystick_buffer_clear(void);
void joystick_add_buffer(KeyboardEvent event);
void joystick_set_axis(Keycode keycode, AnalogValue value);
int joystick_buffer_send(void);

#ifdef __cplusplus
}
#endif

#endif //JOYSTICK_H
