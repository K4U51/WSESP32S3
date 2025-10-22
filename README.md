# WSESP32S3
1.	Background Image — ui_bg
	•	Your custom gauge face PNG.
2.	Dot Object — ui_dot
	•	Small circle or image placed in the center (240,240).
	•	Will move according to G data.
3.	Live Labels —
	•	ui_label_accel (bottom)
	•	ui_label_brake (top)
	•	ui_label_left (left)
	•	ui_label_right (right)
4.	Peak Labels — optional overlay or small text near edges
	•	ui_label_peak_accel
	•	ui_label_peak_brake
	•	ui_label_peak_left
	•	ui_label_peak_right
5.	Gauges (optional) — If you plan to show analog gauges
	•	ui_gauge_accel
	•	ui_gauge_brake
	•	ui_gauge_left
	•	ui_gauge_right

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
