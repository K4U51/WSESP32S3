#pragma once
#include "lvgl.h"

// Gauges
extern lv_obj_t *gauge_accel;
extern lv_obj_t *gauge_brake;
extern lv_obj_t *gauge_left;
extern lv_obj_t *gauge_right;

// Live labels
extern lv_obj_t *label_accel;
extern lv_obj_t *label_brake;
extern lv_obj_t *label_left;
extern lv_obj_t *label_right;

// Peak labels
extern lv_obj_t *label_peak_accel;
extern lv_obj_t *label_peak_brake;
extern lv_obj_t *label_peak_left;
extern lv_obj_t *label_peak_right;

// Dot on dial
extern lv_obj_t *dot_obj;

// Screen init function
void ui_main_screen_init(void);
