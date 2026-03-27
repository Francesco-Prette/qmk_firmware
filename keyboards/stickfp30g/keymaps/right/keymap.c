// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "joystick.h"
#include "action.h"
#include "mousekey.h"
#include "print.h"
#include "analog.h"

#define default 0

/* ---------------------------
   Joystick calibration data
---------------------------*/

typedef struct {
    int16_t center_x;
    int16_t center_y;
    int16_t min_x;
    int16_t max_x;
    int16_t min_y;
    int16_t max_y;
} joystick_calibration_t;

joystick_calibration_t joy;

bool calibration_mode = false;

/* ---------------------------
   Keymap
---------------------------*/

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [default] = LAYOUT_default(
        KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,
        KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
        KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,
        KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        KC_NO, MO(1), KC_MINS, KC_NO, KC_END
    ),

    [1] = LAYOUT_1(
        KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_DEL,
        KC_Y, KC_U, KC_I, KC_LBRC, KC_RBRC, KC_BSLS,
        KC_H, KC_J, KC_K, KC_L, KC_QUOT, KC_ENT,
        KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_HOME,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_END
    )
};

/* ---------------------------
   Load calibration
---------------------------*/

void keyboard_post_init_user(void) {

    eeprom_read_block(&joy, (void*)EECONFIG_USER, sizeof(joy));

    if (joy.center_x == 0) {
        joy.center_x = 512;
        joy.center_y = 512;
        joy.min_x = 0;
        joy.max_x = 1023;
        joy.min_y = 0;
        joy.max_y = 1023;
    }
}

/* ---------------------------
   Save calibration
---------------------------*/

void save_calibration(void) {
    eeprom_update_block(&joy, (void*)EECONFIG_USER, sizeof(joy));
}

/* ---------------------------
   Update calibration
---------------------------*/

void update_calibration(int16_t x, int16_t y) {

    if (x < joy.min_x) joy.min_x = x;
    if (x > joy.max_x) joy.max_x = x;

    if (y < joy.min_y) joy.min_y = y;
    if (y > joy.max_y) joy.max_y = y;
}

/* ---------------------------
   Key processing
---------------------------*/

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (keycode == KC_MINS && record->event.pressed) {

        calibration_mode = !calibration_mode;

        if (calibration_mode) {

            joy.min_x = 1023;
            joy.max_x = 0;
            joy.min_y = 1023;
            joy.max_y = 0;

            joy.center_x = analogReadPin(F5);
            joy.center_y = analogReadPin(F4);

            uprintf("Joystick calibration START\n");

        } else {

            save_calibration();
            uprintf("Joystick calibration SAVED\n");

        }
    }

    return true;
}

#ifdef JOYSTICK_ENABLE

    #define JOYSTICK_DEADZONE 5

    joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
        JOYSTICK_AXIS_IN(F5, 0, 512, 1023),
        JOYSTICK_AXIS_IN(F4, 0, 512, 1023)
    };

    static bool w_pressed = false;
    static bool s_pressed = false;
    static bool a_pressed = false;
    static bool d_pressed = false;

    void process_joystick_input(void) {

        int16_t raw_x = analogReadPin(F5);
        int16_t raw_y = analogReadPin(F4);

        if (calibration_mode) {
            update_calibration(raw_x, raw_y);
        }

        int16_t x_axis = joystick_read_axis(0);
        int16_t y_axis = joystick_read_axis(1);

        if (y_axis > JOYSTICK_DEADZONE) {
            if (!w_pressed) {
                register_code(KC_UP);
                w_pressed = true;
            }
        } else {
            if (w_pressed) {
                unregister_code(KC_UP);
                w_pressed = false;
            }
        }

        if (y_axis < -JOYSTICK_DEADZONE) {
            if (!s_pressed) {
                register_code(KC_DOWN);
                s_pressed = true;
            }
        } else {
            if (s_pressed) {
                unregister_code(KC_DOWN);
                s_pressed = false;
            }
        }

        if (x_axis > JOYSTICK_DEADZONE) {
            if (!d_pressed) {
                register_code(KC_LEFT);
                d_pressed = true;
            }
        } else {
            if (d_pressed) {
                unregister_code(KC_LEFT);
                d_pressed = false;
            }
        }

        if (x_axis < -JOYSTICK_DEADZONE) {
            if (!a_pressed) {
                register_code(KC_RIGHT);
                a_pressed = true;
            }
        } else {
            if (a_pressed) {
                unregister_code(KC_RIGHT);
                a_pressed = false;
            }
        }
    }

    void matrix_scan_user(void) {
        process_joystick_input();
    }

#endif