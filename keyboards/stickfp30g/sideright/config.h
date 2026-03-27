#pragma once

#ifdef OLED_ENABLE
    #define OLED_DISPLAY_128X64
#endif

#ifdef POINTING_DEVICE_ENABLE

    // joystick pins
    #define ANALOG_JOYSTICK_X_AXIS_PIN F5
    #define ANALOG_JOYSTICK_Y_AXIS_PIN F4

    // polling interval
    #define ANALOG_JOYSTICK_READ_INTERVAL 5

    // speed parameters
    #define ANALOG_JOYSTICK_SPEED_REGULATOR 15
    #define ANALOG_JOYSTICK_SPEED_MAX 20

    // invert axis if needed
    #define POINTING_DEVICE_INVERT_X

#endif


#ifdef MOUSEKEY_ENABLE
    #define MOUSEKEY_DELAY          0
    #define MOUSEKEY_INTERVAL       16
    #define MOUSEKEY_MAX_SPEED      10
    #define MOUSEKEY_TIME_TO_MAX    20
    #define MOUSEKEY_WHEEL_DELAY    0
#endif