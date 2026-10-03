/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "color.h"

/* All arithmetic below is single precision on purpose: Cortex-M4F/M7 cores
 * have a float FPU only, and a double literal silently turns an expression
 * into a software double-precision call. */

void rgb_to_hsv(ColorHSV * restrict hsv, const ColorRGB * restrict rgb)
{
    const float r = rgb->r;
    const float g = rgb->g;
    const float b = rgb->b;
    float max = r > g ? r : g;
    float min = r < g ? r : g;
    max = b > max ? b : max;
    min = b < min ? b : min;
    const float chroma = max - min;

    if (chroma == 0.0f)
    {
        hsv->h = 0;
    }
    else if (max == r)
    {
        hsv->h = (uint16_t)(g >= b ? 60.0f * (g - b) / chroma : 60.0f * (g - b) / chroma + 360.0f);
    }
    else if (max == g)
    {
        hsv->h = (uint16_t)(60.0f * (b - r) / chroma + 120.0f);
    }
    else
    {
        hsv->h = (uint16_t)(60.0f * (r - g) / chroma + 240.0f);
    }
    hsv->s = (uint8_t)(max == 0.0f ? 0.0f : 100.0f * (1.0f - min / max));
    hsv->v = (uint8_t)(max * 100.0f / 255.0f);
}

/* Integer HSV to RGB. s and v are percentages and the hue fraction is in
 * sixtieths of a degree, so every divisor is a compile-time constant (which
 * the compiler turns into a multiply): no runtime division and no floating
 * point for an operation that runs once per LED per frame in the hue-driven
 * effects. Results are the floor of the exact value, as before. */
void hsv_to_rgb(ColorRGB * restrict rgb, const ColorHSV * restrict hsv)
{
    const uint32_t v = hsv->v > 100 ? 100 : hsv->v;
    const uint32_t s = hsv->s > 100 ? 100 : hsv->s;
    const uint8_t value = (uint8_t)(v * 255u / 100u);
    if (s == 0)
    {
        rgb->r = value;
        rgb->g = value;
        rgb->b = value;
        return;
    }
    const uint32_t sector = hsv->h / 60u;
    const uint32_t fraction = hsv->h - sector * 60u;    /* 0..59 sixtieths of a degree */
    const uint8_t x = (uint8_t)(v * (100u - s) * 255u / 10000u);
    /* Even sectors use the rising channel, odd sectors the falling channel.
     * Each sector needs only one of them, with the same exact truncation. */
    const uint32_t interpolation = sector & 1u ? fraction : 60u - fraction;
    const uint8_t intermediate = (uint8_t)(v * (6000u - s * interpolation) * 255u / 600000u);
    switch (sector)
    {
        case 0:  rgb->r = value;        rgb->g = intermediate; rgb->b = x;            break;
        case 1:  rgb->r = intermediate; rgb->g = value;        rgb->b = x;            break;
        case 2:  rgb->r = x;            rgb->g = value;        rgb->b = intermediate; break;
        case 3:  rgb->r = x;            rgb->g = intermediate; rgb->b = value;        break;
        case 4:  rgb->r = intermediate; rgb->g = x;            rgb->b = value;        break;
        case 5:  rgb->r = value;        rgb->g = x;            rgb->b = intermediate; break;
        default: rgb->r = 0;     rgb->g = 0;     rgb->b = 0;     break; /* hue out of range */
    }
}

void colorf_set_hsv(ColorFloat * restrict color, const ColorHSV * restrict hsv)
{
    ColorRGB temp_rgb = {0, 0, 0};
    hsv_to_rgb(&temp_rgb, hsv);
    colorf_set_rgb(color, &temp_rgb);
}

void color_get_hsv(const Color* restrict color, ColorHSV* restrict hsv)
{
    rgb_to_hsv(hsv, color);
}

void color_set_hsv(Color* restrict color, const ColorHSV* restrict hsv)
{
    hsv_to_rgb(color, hsv);
}
