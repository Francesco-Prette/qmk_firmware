// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "analog.h"
#include "print.h"
#include "eeprom.h"
#include <math.h>

#define default 0

enum custom_keycodes {
    KC_MOUSE_CALIB = SAFE_RANGE
};

/* ---------------------------
   Keymap
---------------------------*/

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [default] = LAYOUT_default(
        KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,
        KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
        KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,
        KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        TO(2), MO(1), KC_MINS, KC_HOME, KC_END
    ),

    [1] = LAYOUT_1(
        KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_DEL,
        KC_Y, KC_U, KC_I, KC_LBRC, KC_RBRC, KC_BSLS,
        KC_H, KC_J, KC_K, KC_L, KC_QUOT, KC_ENT,
        KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        KC_NO, KC_NO, KC_NO, KC_HOME, KC_END
    ),

    [2] = LAYOUT_2(
        KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,
        KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
        MS_BTN4, MS_BTN1, MS_BTN2, MS_BTN3, MS_BTN5, KC_ENT,
        KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        TO(default), MO(1), KC_MOUSE_CALIB, KC_HOME, KC_END
    )
};

/* ---------------------------
   Joystick calibration data
---------------------------*/
typedef struct {
    int16_t center_x;
    int16_t center_y;
    uint16_t max_pos_x;
    uint16_t max_neg_x;
    uint16_t max_pos_y;
    uint16_t max_neg_y;
    uint16_t max_radius;
} joy_calibration_t;

static joy_calibration_t joy;
static bool calibration_mode = false;
static uint16_t calibration_timer = 0;
static uint32_t max_radius_sq = 1;
static uint16_t max_pos_dx = 1;
static uint16_t max_neg_dx = 1;
static uint16_t max_pos_dy = 1;
static uint16_t max_neg_dy = 1;
static float subpixel_x = 0.0f;
static float subpixel_y = 0.0f;

#define CALIBRATION_TIME_MS 3000
#define JOYSTICK_DEADZONE_PCT 0.01f
#define JOYSTICK_MIN_SPEED 0.05f
#define JOYSTICK_STAGE1_END 0.35f
#define JOYSTICK_STAGE2_END 0.75f
#define JOYSTICK_STAGE1_MAX_FACTOR 0.30f
#define JOYSTICK_STAGE2_ADD_FACTOR 0.45f
#define JOYSTICK_STAGE3_BASE_FACTOR 0.75f
#define JOYSTICK_STAGE3_ADD_FACTOR 0.25f
#define JOYSTICK_SPEED_MULTIPLIER 1.00f
#define JOYSTICK_MAX_VALUE 15.0f

/* ---------------------------
   Load calibration
---------------------------*/
void keyboard_post_init_user(void) {
    eeprom_read_block(&joy, (void*)0, sizeof(joy));
    if ((joy.center_x == 0 && joy.center_y == 0) ||
        joy.max_pos_x < 100 || joy.max_pos_x > 1500 ||
        joy.max_neg_x < 100 || joy.max_neg_x > 1500 ||
        joy.max_pos_y < 100 || joy.max_pos_y > 1500 ||
        joy.max_neg_y < 100 || joy.max_neg_y > 1500) {
        joy.center_x = 512;
        joy.center_y = 512;
        joy.max_pos_x = 480;
        joy.max_neg_x = 480;
        joy.max_pos_y = 480;
        joy.max_neg_y = 480;
        joy.max_radius = 480;
    }
}

/* ---------------------------
   Save calibration
---------------------------*/
void save_calibration(void) {
    eeprom_update_block(&joy, (void*)0, sizeof(joy));
}

/* ---------------------------
   Update calibration
---------------------------*/
void update_calibration(int16_t x, int16_t y) {
    int32_t dx = (int32_t)x - joy.center_x;
    int32_t dy = (int32_t)y - joy.center_y;

    if (dx >= 0) {
        uint16_t pdx = (uint16_t)dx;
        if (pdx > max_pos_dx) {
            max_pos_dx = pdx;
        }
    } else {
        uint16_t ndx = (uint16_t)(-dx);
        if (ndx > max_neg_dx) {
            max_neg_dx = ndx;
        }
    }
    if (dy >= 0) {
        uint16_t pdy = (uint16_t)dy;
        if (pdy > max_pos_dy) {
            max_pos_dy = pdy;
        }
    } else {
        uint16_t ndy = (uint16_t)(-dy);
        if (ndy > max_neg_dy) {
            max_neg_dy = ndy;
        }
    }

    uint32_t r2 = (uint32_t)(dx * dx + dy * dy);
    if (r2 > max_radius_sq) {
        max_radius_sq = r2;
    }
}

/* ---------------------------
   Calibration toggle
---------------------------*/
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_MOUSE_CALIB && record->event.pressed) {
        if (!calibration_mode) {
            calibration_mode  = true;
            calibration_timer = timer_read();
            max_radius_sq     = 1;
            max_pos_dx        = 1;
            max_neg_dx        = 1;
            max_pos_dy        = 1;
            max_neg_dy        = 1;
            joy.center_x      = analogReadPin(ANALOG_JOYSTICK_X_AXIS_PIN);
            joy.center_y      = analogReadPin(ANALOG_JOYSTICK_Y_AXIS_PIN);
        } else {
            calibration_mode = false;
            joy.max_pos_x    = (uint16_t)fmaxf(100.0f, (float)max_pos_dx);
            joy.max_neg_x    = (uint16_t)fmaxf(100.0f, (float)max_neg_dx);
            joy.max_pos_y    = (uint16_t)fmaxf(100.0f, (float)max_pos_dy);
            joy.max_neg_y    = (uint16_t)fmaxf(100.0f, (float)max_neg_dy);
            joy.max_radius   = (uint16_t)fmaxf(100.0f, sqrtf((float)max_radius_sq));
            save_calibration();
        }
        return false;
    }
    return true;
}

/* ---------------------------
   Custom curve in pointing_device
---------------------------*/
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {

    int16_t raw_x = analogReadPin(ANALOG_JOYSTICK_X_AXIS_PIN);
    int16_t raw_y = analogReadPin(ANALOG_JOYSTICK_Y_AXIS_PIN);

    if (calibration_mode) {
        update_calibration(raw_x, raw_y);
        if (timer_elapsed(calibration_timer) >= CALIBRATION_TIME_MS) {
            calibration_mode = false;
            joy.max_pos_x    = (uint16_t)fmaxf(100.0f, (float)max_pos_dx);
            joy.max_neg_x    = (uint16_t)fmaxf(100.0f, (float)max_neg_dx);
            joy.max_pos_y    = (uint16_t)fmaxf(100.0f, (float)max_pos_dy);
            joy.max_neg_y    = (uint16_t)fmaxf(100.0f, (float)max_neg_dy);
            joy.max_radius   = (uint16_t)fmaxf(100.0f, sqrtf((float)max_radius_sq));
            save_calibration();
        }
        return mouse_report;
    }

    int16_t dx = raw_x - joy.center_x;
    int16_t dy = raw_y - joy.center_y;

    float nx      = (dx >= 0) ? (dx / (float)joy.max_pos_x) : (dx / (float)joy.max_neg_x);
    float ny      = (dy >= 0) ? (dy / (float)joy.max_pos_y) : (dy / (float)joy.max_neg_y);
    float raw_mag = sqrtf(nx * nx + ny * ny);

    // Radial deadzone and normalization for smooth 360-degree movement.
    if (raw_mag <= JOYSTICK_DEADZONE_PCT) {
        subpixel_x = 0.0f;
        subpixel_y = 0.0f;
        mouse_report.x = 0;
        mouse_report.y = 0;
        return mouse_report;
    }

    float mag = (raw_mag - JOYSTICK_DEADZONE_PCT) / (1.0f - JOYSTICK_DEADZONE_PCT);
    mag = fmaxf(fminf(mag, 1.0f), 0.0f);

    // 3-stage ramp for more speed steps before hitting max speed.
    float speed_factor = 0.0f;
    if (mag < JOYSTICK_STAGE1_END) {
        float t = mag / JOYSTICK_STAGE1_END;
        speed_factor = JOYSTICK_STAGE1_MAX_FACTOR * t;
    } else if (mag < JOYSTICK_STAGE2_END) {
        float t = (mag - JOYSTICK_STAGE1_END) / (JOYSTICK_STAGE2_END - JOYSTICK_STAGE1_END);
        speed_factor = JOYSTICK_STAGE1_MAX_FACTOR + (JOYSTICK_STAGE2_ADD_FACTOR * t * t);
    } else {
        float t = (mag - JOYSTICK_STAGE2_END) / (1.0f - JOYSTICK_STAGE2_END);
        speed_factor = JOYSTICK_STAGE3_BASE_FACTOR + (JOYSTICK_STAGE3_ADD_FACTOR * t * t * t);
    }

    float speed = speed_factor * JOYSTICK_MAX_VALUE * JOYSTICK_SPEED_MULTIPLIER;
    if (speed < JOYSTICK_MIN_SPEED && raw_mag > 0.01f) {
        speed = JOYSTICK_MIN_SPEED;
    }
    float scale = (raw_mag > 0.0f) ? (speed / raw_mag) : 0.0f;

    float move_x = (-nx * scale) + subpixel_x;
    float move_y = (ny * scale) + subpixel_y;

    int16_t out_x = (int16_t)move_x;
    int16_t out_y = (int16_t)move_y;

    if (out_x > 127) out_x = 127;
    if (out_x < -127) out_x = -127;
    if (out_y > 127) out_y = 127;
    if (out_y < -127) out_y = -127;

    subpixel_x = move_x - (float)out_x;
    subpixel_y = move_y - (float)out_y;

    mouse_report.x = (int8_t)out_x;
    mouse_report.y = (int8_t)out_y;

    return mouse_report;
}

#ifdef OLED_ENABLE

    enum layer_names {
        _L_DEFAULT,
        _L_1,
        _L_2
    };

    bool oled_task_user(void) {
        // RENDER QMK
        //render_logo();
        switch (get_highest_layer(layer_state)){
            case _L_DEFAULT:
                oled_write_ln("Default", false);
                break;
            case _L_1:
                oled_write_ln("Layer 1", false);
                break;
            case _L_2:
                oled_write_ln("Mouse", false);
                break;
        }
        return false;
    }
#endif
