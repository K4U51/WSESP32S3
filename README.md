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
├── main.ino                ← base, modified to include your UI logic
├── LVGL_Driver.cpp
├── LVGL_Driver.h
├── LVGL_Example.cpp        ← you may keep for reference or remove
├── LVGL_Example.h
├── Wireless.cpp
├── Wireless.h
├── Gyro_QMI8658.cpp
├── Gyro_QMI8658.h
├── RTC_PCF85063.cpp
├── RTC_PCF85063.h
├── SD_Card.cpp
├── SD_Card.h
├── BAT_Driver.cpp
├── BAT_Driver.h
├── TCA9554PWR.cpp
├── TCA9554PWR.h
├── PCF85063.cpp
├── PCF85063.h
├── QMI8658.cpp
├── QMI8658.h
├── ST7701S.cpp
├── ST7701S.h
├── CST820.cpp
├── CST820.h
│
├── ui/                      ← new from SquareLine
│   ├── ui.c
│   ├── ui.h
│   ├── ui_helpers.c
│   └── ui_helpers.h
│
├── assets/                  ← new from SquareLine
│   ├── dial_bg.c
│   └── dial_bg.h
│
├── fonts/                   ← optional, if fonts are separate
│   └── montserrat_bold_28.h
│
└── gforce_log.csv           ← (runtime)  
