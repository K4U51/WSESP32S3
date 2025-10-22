# WSESP32S3

Dial background
Image
ui_dial_image

Dot marker
Object / Circle
ui_dot

Left G label
Label
ui_label_left

Right G label
Label
ui_label_right

Accel (forward) label
Label
ui_label_accel

Brake (rearward) label
Label
ui_label_brake

``` 
WSESP32S3/
│
├── main.ino                  ← Base Arduino sketch with main loop, UI init, logging
│
├── LVGL_Driver.cpp           ← LVGL platform/driver layer (TFT, touch)
├── LVGL_Driver.h
├── LVGL_Example.cpp          ← Optional example/test, can remove
├── LVGL_Example.h
│
├── Wireless.cpp              ← Wireless module
├── Wireless.h
│
├── Gyro_QMI8658.cpp          ← IMU driver
├── Gyro_QMI8658.h
│
├── RTC_PCF85063.cpp          ← RTC driver
├── RTC_PCF85063.h
│
├── SD_Card.cpp               ← SD card logging
├── SD_Card.h
│
├── BAT_Driver.cpp            ← Battery monitor
├── BAT_Driver.h
│
├── TCA9554PWR.cpp            ← I/O expander driver
├── TCA9554PWR.h
│
├── PCF85063.cpp              ← RTC lower-level handling
├── PCF85063.h
│
├── QMI8658.cpp               ← IMU lower-level handling
├── QMI8658.h
│
├── ST7701S.cpp               ← TFT display driver
├── ST7701S.h
│
├── CST820.cpp                ← Touch driver
├── CST820.h
│
├── ui/                       ← SquareLine Studio UI export
│   ├── ui.c                  ← LVGL object creation & layout
│   ├── ui.h                  ← Object references (ui_dot, labels, etc.)
│   ├── ui_helpers.c          ← Optional helper functions for UI
│   └── ui_helpers.h
│
├── assets/                   ← Images exported from SquareLine
│   ├── dial_bg.c
│   └── dial_bg.h
│
├── fonts/                    ← Custom fonts (optional)
│   └── montserrat_bold_28.h
│
├── lib/                      ← External libraries
│   └── lvgl/                 ← LVGL library folder
│
└── gforce_log.csv            ← Runtime SD card log file
