#include <gtest/gtest.h>

#include "advanced_key.h"
#include "math.h"

/* A normalizer override consumes this public Q16 scale. Compare every raw
 * span and direction with the wide signed definition, including spans whose
 * quotient uses the high bit when LUT_LENGTH is configured near 65535. */
TEST(AdvancedKeyTest, CalibrationScaleMatchesEveryRawSpan)
{
    AdvancedKey key = {};
    const int64_t numerator = (int64_t)LUT_LENGTH << 16;
    for (uint32_t span = 1; span <= UINT16_MAX; span++)
    {
        advanced_key_set_range(&key, (AnalogRawValue)span, 0);
        ASSERT_EQ((int32_t)(numerator / span), key.q_scale_to_index) << "span=" << span;
        ASSERT_EQ(span, key.config.upper_bound);
        ASSERT_EQ(0, key.config.lower_bound);

        advanced_key_set_range(&key, 0, (AnalogRawValue)span);
        ASSERT_EQ((int32_t)(-numerator / span), key.q_scale_to_index) << "span=" << span;
        ASSERT_EQ(0, key.config.upper_bound);
        ASSERT_EQ(span, key.config.lower_bound);
    }
    advanced_key_set_range(&key, 2048, 2048);
    EXPECT_EQ(0, key.q_scale_to_index);
    EXPECT_EQ(ANALOG_VALUE_MIN, advanced_key_normalize(&key, 0));
    EXPECT_EQ(ANALOG_VALUE_MIN, advanced_key_normalize(&key, UINT16_MAX));
}

TEST(AdvancedKeyTest, DigitalMode)
{
    static AdvancedKey advanced_key =
    {
        .config = 
        {
            .mode = ADVANCED_KEY_DIGITAL_MODE,
        }
    };
    advanced_key_update(&advanced_key, true);
    EXPECT_TRUE(advanced_key.key.state);
    advanced_key_update(&advanced_key, false);
    EXPECT_FALSE(advanced_key.key.state);
}

TEST(AdvancedKeyTest, NormalMode)
{
    static AdvancedKey advanced_key = 
    {
        .config = 
        {
            .mode = ADVANCED_KEY_ANALOG_NORMAL_MODE,
            .activation_value = A_ANTI_NORM(0.50),
            .deactivation_value = A_ANTI_NORM(0.49),
        },
    };
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.2));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.6));
    EXPECT_TRUE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.4));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.8));
    EXPECT_TRUE(advanced_key.key.state);
}

TEST(AdvancedKeyTest, RapidTriggerMode)
{
    static AdvancedKey advanced_key = 
    {
        .config =
        {
            .mode = ADVANCED_KEY_ANALOG_RAPID_MODE,
            .trigger_distance = A_ANTI_NORM(0.08),
            .release_distance = A_ANTI_NORM(0.08),
            .upper_deadzone = A_ANTI_NORM(0.10),
            .lower_deadzone = A_ANTI_NORM(0.20),
        },
    };
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.09));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.12));
    EXPECT_TRUE(advanced_key.key.state);
    EXPECT_EQ(advanced_key.extremum, A_ANTI_NORM(0.12));
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.60));
    EXPECT_TRUE(advanced_key.key.state);
    EXPECT_EQ(advanced_key.extremum, A_ANTI_NORM(0.60));
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.50));
    EXPECT_FALSE(advanced_key.key.state);
    EXPECT_EQ(advanced_key.extremum, A_ANTI_NORM(0.50));
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.60));
    EXPECT_TRUE(advanced_key.key.state);
    EXPECT_EQ(advanced_key.extremum, A_ANTI_NORM(0.60));
    advanced_key_update(&advanced_key, A_ANTI_NORM(1.00));
    EXPECT_TRUE(advanced_key.key.state);
    EXPECT_EQ(advanced_key.extremum, A_ANTI_NORM(1.00));
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.82));
    EXPECT_TRUE(advanced_key.key.state);
    EXPECT_EQ(advanced_key.extremum, A_ANTI_NORM(1.00));
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.78));
    EXPECT_FALSE(advanced_key.key.state);
    EXPECT_EQ(advanced_key.extremum, A_ANTI_NORM(0.78));
}

// Expected difference between two normalized travels, computed the same way the
// library does it (difference of the converted values) so it stays well defined
// for negative deltas; a negative float cast to AnalogValue is undefined.
#define A_DIFF(a, b) ((int32_t)A_ANTI_NORM(a) - (int32_t)A_ANTI_NORM(b))

TEST(AdvancedKeyTest, SpeedMode)
{
    static AdvancedKey advanced_key = 
    {
        .config =
        {
            .mode = ADVANCED_KEY_ANALOG_SPEED_MODE,
            .trigger_speed = A_ANTI_NORM(0.04),
            .release_speed = A_ANTI_NORM(0.04),
            .upper_deadzone = A_ANTI_NORM(0.10),
            .lower_deadzone = A_ANTI_NORM(0.20),
        },
    };
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.09));
    EXPECT_NEAR(advanced_key.difference, A_ANTI_NORM(0.09),A_ANTI_NORM(1e-4));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.12));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.12, 0.09), A_ANTI_NORM(1e-4));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.20));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.20, 0.12), A_ANTI_NORM(1e-4));
    EXPECT_TRUE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.60));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.60, 0.20), A_ANTI_NORM(1e-4));
    EXPECT_TRUE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.80));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.80, 0.60), A_ANTI_NORM(1e-4));
    EXPECT_TRUE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.78));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.78, 0.80), A_ANTI_NORM(1e-4));
    EXPECT_TRUE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.72));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.72, 0.78), A_ANTI_NORM(1e-4));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.74));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.74, 0.72), A_ANTI_NORM(1e-4));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.76));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.76, 0.74), A_ANTI_NORM(1e-4));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.78));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.78, 0.76), A_ANTI_NORM(1e-4));
    EXPECT_FALSE(advanced_key.key.state);
    advanced_key_update(&advanced_key, A_ANTI_NORM(0.81));
    EXPECT_NEAR(advanced_key.difference, A_DIFF(0.81, 0.78), A_ANTI_NORM(1e-4));
    EXPECT_TRUE(advanced_key.key.state);
}

TEST(AdvancedKeyTest, Value)
{
    static AdvancedKey advanced_key;
    for (int i = 0; i <= ADVANCED_KEY_ANALOG_SPEED_MODE; i++)
    {
        advanced_key.config.mode = i;
        for (int j = 0; j < 1000; j++)
        {
            float value = A_ANTI_NORM((-cos(j/100.f)*0.5+0.5)); // stays within [0, 1]
            advanced_key_update(&advanced_key, value);
            EXPECT_EQ(advanced_key.value, value);
        }
    }
}

TEST(AdvancedKeyTest, Calibration)
{
    const float default_upper_bound = 2048;

    {
        static AdvancedKey advanced_key = 
        {
            .config = 
            {
                .mode = ADVANCED_KEY_ANALOG_NORMAL_MODE,
                .calibration_mode = ADVANCED_KEY_AUTO_CALIBRATION_UNDEFINED,
                .activation_value = A_ANTI_NORM(0.50),
                .deactivation_value = A_ANTI_NORM(0.49),
                .upper_bound = (AnalogRawValue)default_upper_bound,
            },
        };
        advanced_key_update_raw(&advanced_key, default_upper_bound+DEFAULT_ESTIMATED_RANGE-100);
        advanced_key_update_raw(&advanced_key, default_upper_bound+DEFAULT_ESTIMATED_RANGE+100);
        EXPECT_EQ(advanced_key.config.calibration_mode, ADVANCED_KEY_AUTO_CALIBRATION_POSITIVE);
        EXPECT_EQ(advanced_key.config.lower_bound, default_upper_bound+DEFAULT_ESTIMATED_RANGE+100);
        advanced_key_update_raw(&advanced_key, default_upper_bound+DEFAULT_ESTIMATED_RANGE+500);
        EXPECT_EQ(advanced_key.config.calibration_mode, ADVANCED_KEY_AUTO_CALIBRATION_POSITIVE);
        EXPECT_EQ(advanced_key.config.lower_bound, default_upper_bound+DEFAULT_ESTIMATED_RANGE+500);
    }
    {
        static AdvancedKey advanced_key = 
        {
            .config = 
            {
                .mode = ADVANCED_KEY_ANALOG_NORMAL_MODE,
                .calibration_mode = ADVANCED_KEY_AUTO_CALIBRATION_UNDEFINED,
                .activation_value = A_ANTI_NORM(0.50),
                .deactivation_value = A_ANTI_NORM(0.49),
                .upper_bound = (AnalogRawValue)default_upper_bound,
            },
        };
        advanced_key_update_raw(&advanced_key, default_upper_bound-DEFAULT_ESTIMATED_RANGE+100);
        advanced_key_update_raw(&advanced_key, default_upper_bound-DEFAULT_ESTIMATED_RANGE-100);
        EXPECT_EQ(advanced_key.config.calibration_mode, ADVANCED_KEY_AUTO_CALIBRATION_NEGATIVE);
        EXPECT_EQ(advanced_key.config.lower_bound, default_upper_bound-DEFAULT_ESTIMATED_RANGE-100);
        advanced_key_update_raw(&advanced_key, default_upper_bound-DEFAULT_ESTIMATED_RANGE-500);
        EXPECT_EQ(advanced_key.config.calibration_mode, ADVANCED_KEY_AUTO_CALIBRATION_NEGATIVE);
        EXPECT_EQ(advanced_key.config.lower_bound, default_upper_bound-DEFAULT_ESTIMATED_RANGE-500);
    }
}
