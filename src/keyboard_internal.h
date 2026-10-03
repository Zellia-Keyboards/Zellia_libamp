/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KEYBOARD_INTERNAL_H_
#define KEYBOARD_INTERNAL_H_

#include "keyboard.h"
#include "layer.h"

/* Debounce the physical state the scan just wrote into the key, publish the
 * resulting report state, and hand the key's bound keycode to the event
 * handler. Returns whether the report state changed. Unchanged keys skip the
 * handler call entirely unless their binding needs refreshing on every poll
 * (see keyboard_event_needs_dispatch()); this is the per-key, per-tick fast path. */
static inline bool keyboard_key_report_and_dispatch(Key *key)
{
    const bool changed = keyboard_key_set_report_state(key, keyboard_key_debounce(key));
    const Keycode keycode = layer_cache_get_keycode(key->id);
    if (changed || keyboard_keycode_dispatches_unchanged(keycode))
    {
        /* Most polls have nothing to dispatch. Keep event construction on
         * this branch so Cortex-M does not spill an unused event to stack. */
        keyboard_event_handler(MK_EVENT(keycode, EVENT_TYPE(changed, key->report_state), key));
    }
    return changed;
}

/* Nexus has already read the key to decide whether it needs processing. Share
 * the implementation so its loop can reuse those loads; public callers keep
 * keyboard_key_update(), and callback/event ordering is the same. */
#if defined(__GNUC__)
__attribute__((always_inline))
#endif
static inline bool keyboard_update_key_state(Key *key, bool state)
{
    key_update(key, state);
    return keyboard_key_report_and_dispatch(key);
}

#endif /* KEYBOARD_INTERNAL_H_ */
