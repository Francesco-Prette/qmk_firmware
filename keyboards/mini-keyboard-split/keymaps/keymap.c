// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┬───┬───┐
     * │ESC│ 1 │ 2 │ 3 │ 4 │ 5 │
     * ├───┼───┼───┼───┼───┼───┤
     * │TAB│ q │ w │ e │ r │ t │
     * ├───┼───┼───┼───┼───┼───┤
     * │SHI│ a │ s │ d │ f │ g │
     * ├───┼───┼───┼───┼───┼───┤
     * │SHI│ z │ x │ c │ v │ b │
     * ├───┼───┼───┼───┼───┼───┘
     * │CTL│Win│ALT│FN2│SPA│
     * └───┴───┴───┴───┴───┘
     */
    [default] = LAYOUT_default(
        KC_ESC,         KC_1,    KC_2,          KC_3,    KC_4,    KC_5,
        KC_TAB,         KC_Q,    KC_W,          KC_E,    KC_R,    KC_T,
        KC_RIGHT_SHIFT,   KC_A,    KC_S,          KC_D,    KC_F,    KC_G,
        KC_LEFT_SHIFT,  KC_Z,    KC_X,          KC_C,    KC_V,    KC_B,
        KC_LEFT_CTRL,   KC_LWIN, KC_LEFT_ALT,   MO(1),   KC_SPACE
    )
}
