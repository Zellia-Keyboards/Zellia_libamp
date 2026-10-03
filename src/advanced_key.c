/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "advanced_key.h"
#include "keyboard_def.h"
#include "analog.h"
#include "advanced_key_internal.h"
#include "analog_internal.h"

_Static_assert(LUT_LENGTH > 0 && LUT_LENGTH <= 65535, "LUT_LENGTH must fit the AnalogValue range");

#ifdef CALIBRATION_LPF_ENABLE
AnalogRawValue libamp_calibration_low_pass_raws[ADVANCED_KEY_NUM];
#endif

static void advanced_key_update_effective_scale(AdvancedKey *advanced_key);

void advanced_key_init(AdvancedKey* advanced_key, uint16_t id)
{
    key_init(&advanced_key->key, id);
    advanced_key->value = 0;
    advanced_key->raw = 0;
    advanced_key->filtered_raw = 0;
    advanced_key->extremum = 0;
    advanced_key->difference = 0;
    advanced_key->q_scale_effective = 0; /* recomputed on first use */
}


bool advanced_key_update(AdvancedKey* advanced_key, AnalogValue value)
{
    return advanced_key_update_value(advanced_key, value);
}

bool advanced_key_update_raw(AdvancedKey* advanced_key, AnalogRawValue raw)
{
    return advanced_key_update_sample(advanced_key, raw);
}

bool advanced_key_update_state(AdvancedKey* advanced_key, bool state)
{
    return key_update(&(advanced_key->key), state);
}

int32_t advanced_key_lut_index(const AdvancedKey* advanced_key, AnalogRawValue value)
{
    const int32_t delta = (int32_t)advanced_key->config.upper_bound - (int32_t)value;
    /* 64-bit product: delta * q_scale_to_index reaches LUT_LENGTH << 16, which
     * overflows int32 for LUT_LENGTH > 32767 (the default is ANALOG_VALUE_MAX). */
    int32_t index = (int32_t)(((int64_t)delta * advanced_key->q_scale_to_index) >> 16);
    if (index < 0)
    {
        return 0;
    }
    if (index > LUT_LENGTH)
    {
        return LUT_LENGTH;
    }
    return index;
}

/* Linear transfer function: map the calibrated raw range onto the full
 * normalized range. The lookup-table index is rescaled so the result does not
 * depend on LUT_LENGTH; for a power-of-two LUT_LENGTH this is a multiply and a
 * shift, and when LUT_LENGTH equals ANALOG_VALUE_RANGE it folds away entirely. */
__WEAK AnalogValue advanced_key_normalize(AdvancedKey* advanced_key, AnalogRawValue value)
{
    const uint32_t index = (uint32_t)advanced_key_lut_index(advanced_key, value);
    return (AnalogValue)(index * (uint32_t)ANALOG_VALUE_RANGE / (uint32_t)LUT_LENGTH) + ANALOG_VALUE_MIN;
}

void advanced_key_set_range(AdvancedKey* advanced_key, AnalogRawValue upper, AnalogRawValue lower)
{
    advanced_key_set_range_value(advanced_key, upper, lower);
}

void advanced_key_reset_range(AdvancedKey* advanced_key, AnalogRawValue value)
{
    switch (advanced_key->config.calibration_mode)
    {
    case ADVANCED_KEY_AUTO_CALIBRATION_POSITIVE:
        advanced_key_set_range(advanced_key, value, value+DEFAULT_ESTIMATED_RANGE);
        break;
    case ADVANCED_KEY_AUTO_CALIBRATION_NEGATIVE:
        advanced_key_set_range(advanced_key, value, value-DEFAULT_ESTIMATED_RANGE);
        break;
    default:
        advanced_key_set_range(advanced_key, value, value-DEFAULT_ESTIMATED_RANGE);
        break;
    }
}

void advanced_key_set_deadzone(AdvancedKey* advanced_key, AnalogValue upper, AnalogValue lower)
{
    advanced_key->config.upper_deadzone = upper;
    advanced_key->config.lower_deadzone = lower;
    advanced_key_update_effective_scale(advanced_key);
}

__WEAK AnalogRawValue advanced_key_read_raw(AdvancedKey *advanced_key)
{
    return ringbuf_average(&g_adc_ringbufs[g_analog_map[advanced_key->key.id]]);
}

/* Recompute the Q16 effective-travel factor for the current dead zones. The
 * one division of the dead-zone mapping lives here, off the per-frame path. */
static void advanced_key_update_effective_scale(AdvancedKey *advanced_key)
{
    const uint32_t deadzone_sum = (uint32_t)advanced_key->config.upper_deadzone + advanced_key->config.lower_deadzone;
    advanced_key->q_scale_effective_deadzone = deadzone_sum;
    advanced_key->q_scale_effective = deadzone_sum < ANALOG_VALUE_RANGE
        ? ((uint32_t)ANALOG_VALUE_RANGE << 16) / ((uint32_t)ANALOG_VALUE_RANGE - deadzone_sum)
        : 0;
}

/* Travel with the dead zones removed, stretched back over the full range. */
AnalogValue advanced_key_get_effective_value(AdvancedKey *advanced_key)
{
    const int32_t travel = (int32_t)advanced_key->value - (int32_t)ANALOG_VALUE_MIN;
    const int32_t upper_deadzone = advanced_key->config.upper_deadzone;
    const int32_t lower_deadzone = advanced_key->config.lower_deadzone;
    if (travel <= upper_deadzone)
    {
        return ANALOG_VALUE_MIN;
    }
    if (travel >= (int32_t)ANALOG_VALUE_RANGE - lower_deadzone)
    {
        return ANALOG_VALUE_MAX;
    }
    /* The dead zones may be written directly (host packets, stored profiles,
     * user code), so the cached factor is checked against them here. */
    if (advanced_key->q_scale_effective == 0 ||
        advanced_key->q_scale_effective_deadzone != (uint32_t)upper_deadzone + (uint32_t)lower_deadzone)
    {
        advanced_key_update_effective_scale(advanced_key);
        if (advanced_key->q_scale_effective == 0)
        {
            return ANALOG_VALUE_MAX; /* dead zones cover the whole range */
        }
    }
    const uint32_t active_travel = (uint32_t)(travel - upper_deadzone);
    return (AnalogValue)(((uint64_t)active_travel * advanced_key->q_scale_effective) >> 16) + ANALOG_VALUE_MIN;
}
